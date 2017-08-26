#include "ChatSession.h"

ChatSession::ChatSession()
{
}


ChatSession::~ChatSession()
{
}

void ChatSession::OnConnect(void)
{

}

void ChatSession::OnClose(void)
{

}

void ChatSession::OnError(UINT32 dwErrorNo)
{

}

void ChatSession::Release(void)
{
	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void ChatSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

FxSession* ChatSessionManager::CreateSession()
{
	return m_poolSessions.FetchObj();
}

void ChatSessionManager::Init()
{
	m_poolSessions.Init(64, 64, false, 128);
}

void ChatSessionManager::Release(FxSession* pSession)
{
	Assert(0);
	//m_poolSessions.ReleaseObj()
}

void ChatSessionManager::Release(ChatSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}
