#include "ChatServerSession.h"

ChatServerSession::ChatServerSession()
	: m_dwChatPort(0)
	, m_dwChatServerPort(0)
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::OnConnect(void)
{

}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatServerSession::Release(void)
{
}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

//----------------------------------------------------------------------
FxSession* ChatServerSessionManager::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	for (int i = 0; i < ChatConstant::ChatServerNum; ++i)
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
