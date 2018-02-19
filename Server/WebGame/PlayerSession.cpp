#include "PlayerSession.h"
#include "netstream.h"
//#include "gamedefine.h"

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

CPlayerSession::CPlayerSession()
{
}

CPlayerSession::~CPlayerSession()
{
}

void CPlayerSession::OnConnect(void)
{

}

void CPlayerSession::OnClose(void)
{

}

void CPlayerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CPlayerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	//CNetStream oStream(pBuf, dwLen);
	//Protocol::EGameProtocol eProrocol;
	//oStream.ReadInt((int&)eProrocol);
	//const char* pData = pBuf + sizeof(UINT32);
	//dwLen -= sizeof(UINT32);

	//switch (eProrocol)
	//{
	//case Protocol::PLAYER_REQUEST_GAME_MANAGER_INFO:			OnRequestGameManagerInfo(pData, dwLen);	break;
	//default:	Assert(0);	break;
	//}
}

void CPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CPlayerSession::OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen)
{
		//CNetStream oStream(pBuf, dwLen);
		//stPLAYER_REQUEST_GAME_MANAGER_INFO oPLAYER_REQUEST_GAME_MANAGER_INFO;
		//oPLAYER_REQUEST_GAME_MANAGER_INFO.Read(oStream);


}

//////////////////////////////////////////////////////////////////////////
CWebSocketPlayerSession::CWebSocketPlayerSession()
{
}

CWebSocketPlayerSession::~CWebSocketPlayerSession()
{
}

void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
WebSocketPlayerSessionManager::WebSocketPlayerSessionManager()
{
}

WebSocketPlayerSessionManager::~WebSocketPlayerSessionManager()
{
}

FxSession * WebSocketPlayerSessionManager::CreateSession()
{
	FxSession* pSession = NULL;
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (m_oWebSocketPlayerSessions[i].GetConnection() == NULL)
		{
			m_oWebSocketPlayerSessions[i].Init((FxConnection*)0xFFFFFFFF);
			pSession = &m_oWebSocketPlayerSessions[i];
			break;
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (&m_oWebSocketPlayerSessions[i] == pSession)
		{
			m_oWebSocketPlayerSessions[i].Init(NULL);
			break;
		}
	}
	m_oLock.UnLock();
}
