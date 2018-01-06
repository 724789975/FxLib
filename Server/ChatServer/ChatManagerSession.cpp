#include "ChatManagerSession.h"
#include "chatdefine.h"
#include "netstream.h"
#include "ChatServer.h"
#include "ChatServerSession.h"

const static unsigned int g_dwChatServerManagerSessionBuffLen = 64 * 1024;
static char g_pChatServerManagerSessionBuf[g_dwChatServerManagerSessionBuffLen];

ChatManagerSession::ChatManagerSession()
{
}

ChatManagerSession::~ChatManagerSession()
{
}

void ChatManagerSession::OnConnect(void)
{
	LogExe(LogLv_Debug, "remote ip : %s, remote port : %d", GetRemoteIPStr(), GetRemotePort());
	stCHAT_SEND_CHAT_MANAGER_INFO oCHAT_SEND_CHAT_MANAGER_INFO;
	oCHAT_SEND_CHAT_MANAGER_INFO.m_szChatIp = ChatServer::Instance()->GetChatSessionIp();
	oCHAT_SEND_CHAT_MANAGER_INFO.m_dwChatPort = ChatServer::Instance()->GetChatSessionPort();
	oCHAT_SEND_CHAT_MANAGER_INFO.m_dwWebSocketChatPort = ChatServer::Instance()->GetChatWebSocketSessionPort();
	oCHAT_SEND_CHAT_MANAGER_INFO.m_dwChatServerPort = ChatServer::Instance()->GetChatServerSessionPort();
	CNetStream oStream(ENetStreamType_Write, g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_MANAGER_INFO);
	oCHAT_SEND_CHAT_MANAGER_INFO.Write(oStream);
	Send(g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen - oStream.GetDataLength());
}

void ChatManagerSession::OnClose(void)
{
	LogExe(LogLv_Error, "will close!!!!!!!!!");
	extern bool g_bRun;
	g_bRun = false;
}

void ChatManagerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatManagerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oNetStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_INFO:			OnNotifyChatInfo(pData, dwLen);		break;
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_BROADCAST:		OnBroadcastMsg(pData, dwLen);		break;
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_LOGIN:			OnLoginSign(pData, dwLen);	break;
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM:		OnLoginSignByGM(pData, dwLen);	break;
		default:	Assert(0);	break;
	}
}

void ChatManagerSession::Release(void)
{

}

void ChatManagerSession::OnNotifyChatInfo(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_MANAGER_NOTIFY_CHAT_INFO oCHAT_MANAGER_NOTIFY_CHAT_INFO;
	oCHAT_MANAGER_NOTIFY_CHAT_INFO.Read(oStream);

	ChatServer::Instance()->SetHashIndex(oCHAT_MANAGER_NOTIFY_CHAT_INFO.dwHashIndex);

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		LogExe(LogLv_Error, "%s", "FxNetGetModule() == NULL");
		return;
	}
	for (unsigned int i = 0; i < oCHAT_MANAGER_NOTIFY_CHAT_INFO.vecRemoteInfo.size(); ++i)
	{
		FxSession* pChatServerSession = ChatServer::Instance()->GetChatServerSessionManager().CreateSession();
		if (!pChatServerSession)
		{
			LogExe(LogLv_Error, "%s", "can't create ChatServerSession");
			return;
		}
		stCHAT_MANAGER_NOTIFY_CHAT_INFO::stRemoteChatInfo& refInfo = oCHAT_MANAGER_NOTIFY_CHAT_INFO.vecRemoteInfo[i];
		pNet->TcpConnect(pChatServerSession, refInfo.dwIp, refInfo.dwPort, true);
		//ChatServer::Instance()->GetChatServerSessionManager().SetHashIndex(refInfo.dwHashIndex, pChatServerSession);
	}
}

void ChatManagerSession::OnBroadcastMsg(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_MANAGER_NOTIFY_CHAT_BROADCAST oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST;
	if (!oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.Read(oStream))
	{
		LogExe(LogLv_Critical, "error read msg");
		return;
	}

	ChatServer::Instance()->GetChatPlayerManager().OnBroadCastMsg(oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.eChatType, oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.szContent);

}

void ChatManagerSession::OnLoginSign(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_MANAGER_NOTIFY_CHAT_LOGIN oCHAT_MANAGER_NOTIFY_CHAT_LOGIN;
	if (!oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.Read(oStream))
	{
		LogExe(LogLv_Critical, "error read msg");
		return;
	}

	std::string szSign = ChatServer::Instance()->GetChatPlayerManager().CreateLoginSign(oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szPlayerId);

	stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szPlayerId = oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szPlayerId;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szServerId = oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szServerId;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szSign = szSign;

	CNetStream oStreamSign(ENetStreamType_Write, g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen);
	oStreamSign.WriteInt(Protocol::CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN);
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.Write(oStreamSign);
	Send(g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen - oStreamSign.GetDataLength());
}

void ChatManagerSession::OnLoginSignByGM(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_MANAGER_NOTIFY_CHAT_LOGIN oCHAT_MANAGER_NOTIFY_CHAT_LOGIN;
	if (!oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.Read(oStream))
	{
		LogExe(LogLv_Critical, "error read msg");
		return;
	}

	std::string szSign = ChatServer::Instance()->GetChatPlayerManager().CreateLoginSign(oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szPlayerId);

	stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.szPlayerId = oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szPlayerId;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.szSign = szSign;

	CNetStream oStreamSign(ENetStreamType_Write, g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen);
	oStreamSign.WriteInt(Protocol::CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN_GM);
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.Write(oStreamSign);
	Send(g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen - oStreamSign.GetDataLength());
}
