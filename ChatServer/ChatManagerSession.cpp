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
	oCHAT_SEND_CHAT_MANAGER_INFO.m_dwChatServerPort = ChatServer::Instance()->GetChatServerSessionPort();
	CNetStream oStream(ENetStreamType_Write, g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_SEND_CHAT_MANAGER_INFO);
	oCHAT_SEND_CHAT_MANAGER_INFO.Write(oStream);
	Send(g_pChatServerManagerSessionBuf, g_dwChatServerManagerSessionBuffLen - oStream.GetDataLength());
}

void ChatManagerSession::OnClose(void)
{

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
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_INFO:	OnNotifyChatInfo(pData, dwLen);	break;
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

	ChatServer::Instance()->GetChatServerSessionManager().SetHashIndex(oCHAT_MANAGER_NOTIFY_CHAT_INFO.dwHashIndex);

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		LogExe(LogLv_Error, "%s", "FxNetGetModule() == NULL");
		return;
	}
	for (unsigned int i = 0; i < oCHAT_MANAGER_NOTIFY_CHAT_INFO.vecRemoteInfo.size(); ++i)
	{
		ChatServerSession* pChatServerSession = ChatServer::Instance()->GetChatServerSessionManager().GetChatServerSession();
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
