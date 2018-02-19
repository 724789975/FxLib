#include "ServerSession.h"
#include "netstream.h"
#include "gamedefine.h"

const static unsigned int g_dwServerSessionBuffLen = 64 * 1024;
static char g_pServerSessionBuf[g_dwServerSessionBuffLen];

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
	CNetStream oStream(pBuf, dwLen);
	Protocol::EGameProtocol eProrocol;
	oStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
	case Protocol::GAME_NOTIFY_GAME_MANAGER_INFO:			OnGameNotifyGameManagerInfo(pData, dwLen);	break;
	default:	Assert(0);	break;
	}
}

void CServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CServerSession::OnGameNotifyGameManagerInfo(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stGAME_NOTIFY_GAME_MANAGER_INFO oGAME_NOTIFY_GAME_MANAGER_INFO;
	oGAME_NOTIFY_GAME_MANAGER_INFO.Read(oStream);
}

//////////////////////////////////////////////////////////////////////////
CBinaryServerSession::CBinaryServerSession()
{
}

CBinaryServerSession::~CBinaryServerSession()
{
}

void CBinaryServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
FxSession * BinaryServerSessionManager::CreateSession()
{
	CBinaryServerSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool BinaryServerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryServerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryServerSessionManager::Release(CBinaryServerSession * pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

