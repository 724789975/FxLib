#include "SlaveServerSession.h"
#include "netstream.h"
#include "gamedefine.h"

const static unsigned int g_dwSlaveServerSessionBuffLen = 64 * 1024;
static char g_pSlaveServerSessionBuf[g_dwSlaveServerSessionBuffLen];

CSlaveServerSession::CSlaveServerSession()
{
}


CSlaveServerSession::~CSlaveServerSession()
{
}

void CSlaveServerSession::OnConnect(void)
{
}

void CSlaveServerSession::OnClose(void)
{
}

void CSlaveServerSession::OnError(UINT32 dwErrorNo)
{
}

void CSlaveServerSession::OnRecv(const char * pBuf, UINT32 dwLen)
{
}

void CSlaveServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CWebSocketSlaveServerSession::CWebSocketSlaveServerSession()
{

}

CWebSocketSlaveServerSession::~CWebSocketSlaveServerSession()
{

}

void CWebSocketSlaveServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
WebSocketSlaveServerSessionManager::WebSocketSlaveServerSessionManager()
{
}

WebSocketSlaveServerSessionManager::~WebSocketSlaveServerSessionManager()
{
}

FxSession * WebSocketSlaveServerSessionManager::CreateSession()
{
	FxSession* pSession = NULL;
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (m_oWebSocketSlaveServerSessions[i].GetConnection() == NULL)
		{
			m_oWebSocketSlaveServerSessions[i].Init((FxConnection*)0xFFFFFFFF);
			pSession = &m_oWebSocketSlaveServerSessions[i];
			break;
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void WebSocketSlaveServerSessionManager::Release(FxSession * pSession)
{
}

void WebSocketSlaveServerSessionManager::Release(CWebSocketSlaveServerSession* pSession)
{
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (&m_oWebSocketSlaveServerSessions[i] == pSession)
		{
			m_oWebSocketSlaveServerSessions[i].Init(NULL);
			break;
		}
	}
	m_oLock.UnLock();
}


