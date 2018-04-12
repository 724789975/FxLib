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
	m_poolSessions.Init(4, 4);
}

WebSocketSlaveServerSessionManager::~WebSocketSlaveServerSessionManager()
{
}

CWebSocketSlaveServerSession * WebSocketSlaveServerSessionManager::CreateSession()
{
	CWebSocketSlaveServerSession* pSession = NULL;
	pSession = m_poolSessions.FetchObj();
	return pSession;
}

void WebSocketSlaveServerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketSlaveServerSessionManager::Release(CWebSocketSlaveServerSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}


