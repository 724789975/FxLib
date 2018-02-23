#include "ServerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"

const static unsigned int g_dwSlaveServerSessionBuffLen = 64 * 1024;
static char g_pServerSessionBuf[g_dwSlaveServerSessionBuffLen];

CServerSession::CServerSession()
{
}


CServerSession::~CServerSession()
{
}

void CServerSession::OnConnect(void)
{

}

void CServerSession::OnClose(void)
{

}

void CServerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), pBuf);
	if (!Send(pBuf, dwLen))
	{
		LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s send error", GetRemoteIPStr(), GetRemotePort(), pBuf);
		Close();
	}
	//CNetStream oStream(pBuf, dwLen);
	//Protocol::EGameProtocol eProrocol;
	//oStream.ReadInt((int&)eProrocol);
	//const char* pData = pBuf + sizeof(UINT32);
	//dwLen -= sizeof(UINT32);

	//switch (eProrocol)
	//{
	//case Protocol::GAME_NOTIFY_GAME_MANAGER_INFO:			OnGameNotifyGameManagerInfo(pData, dwLen);	break;
	//default:	Assert(0);	break;
	//}
}

void CServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//void CServerSession::OnGameNotifyGameManagerInfo(const char* pBuf, UINT32 dwLen)
//{
//	CNetStream oStream(pBuf, dwLen);
//	stGAME_NOTIFY_GAME_MANAGER_INFO oGAME_NOTIFY_GAME_MANAGER_INFO;
//	oGAME_NOTIFY_GAME_MANAGER_INFO.Read(oStream);
//}

//////////////////////////////////////////////////////////////////////////

CWebSocketServerSession::CWebSocketServerSession()
{
}

CWebSocketServerSession::~CWebSocketServerSession()
{
}

void CWebSocketServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);

	GameServer::Instance()->GetWebSocketServerSessionManager().Release(this);
}

//////////////////////////////////////////////////////////////////////////
WebSocketServerSessionManager::WebSocketServerSessionManager()
{
	m_pCreated = NULL;
}

WebSocketServerSessionManager::~WebSocketServerSessionManager()
{
}

FxSession * WebSocketServerSessionManager::CreateSession()
{
	if (m_pCreated)
	{
		return NULL;
	}
	m_pCreated = &m_oWebSocketSlaveServerSession;
	return m_pCreated;
}

void WebSocketServerSessionManager::Release(FxSession * pSession)
{
	m_pCreated = NULL;
}
