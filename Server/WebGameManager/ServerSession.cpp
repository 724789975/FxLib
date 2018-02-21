#include "ServerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"

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

	stGAME_MANAGER_ACK_GAME_INFO_RESULT oGAME_MANAGER_ACK_GAME_INFO_RESULT;
	oGAME_MANAGER_ACK_GAME_INFO_RESULT.dwResult = 1;
	CPlayerSession* pPlayer = (CPlayerSession*)oGAME_NOTIFY_GAME_MANAGER_INFO.qwPlayerPoint;
	if (GameServer::Instance()->DelRequestPlayer(pPlayer))
	{
		oGAME_MANAGER_ACK_GAME_INFO_RESULT.dwResult = 0;
		pPlayer->OnGameInfo(oGAME_NOTIFY_GAME_MANAGER_INFO);
	}

	CNetStream oWriteStream(ENetStreamType_Write, g_pServerSessionBuf, g_dwServerSessionBuffLen);
	oWriteStream.WriteInt(Protocol::GAME_MANAGER_ACK_GAME_INFO_RESULT);

	oGAME_MANAGER_ACK_GAME_INFO_RESULT.Write(oWriteStream);
	Send(g_pServerSessionBuf, g_dwServerSessionBuffLen - oWriteStream.GetDataLength());
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

