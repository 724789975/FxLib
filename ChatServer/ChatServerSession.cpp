#include "ChatServerSession.h"

ChatServerSession::ChatServerSession()
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::OnConnect(void)
{
	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

void ChatServerSession::Release(void)
{
	OnDestroy();
	//Init(NULL);
	//ChatServerSessionManager::Instance()->Release(this);
}


FxSession* ChatServerSessionManager::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	for (int i = 0; i < ChatConstant::g_dwChatServerNum; ++i)
	{
		if (m_oChatServerSessions[i].GetConnection() == NULL)
		{
			pSession = &m_oChatServerSessions[i];
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void ChatServerSessionManager::Release(FxSession* pSession)
{

}
