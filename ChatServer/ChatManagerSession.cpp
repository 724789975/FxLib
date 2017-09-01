#include "ChatManagerSession.h"
#include "chatdefine.h"
#include "netstream.h"
#include "ChatServer.h"
#include "ChatServerSession.h"


ChatManagerSession::ChatManagerSession()
{
}


ChatManagerSession::~ChatManagerSession()
{
}

void ChatManagerSession::OnConnect(void)
{

}

void ChatManagerSession::OnClose(void)
{

}

void ChatManagerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatManagerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	Protocol::EChatProtocol eProrocol = (Protocol::EChatProtocol)(*((UINT32*)pBuf));
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
		ChatServer::Instance()->GetChatServerSessionManager().SetHashIndex(refInfo.dwHashIndex, pChatServerSession);
	}
}
