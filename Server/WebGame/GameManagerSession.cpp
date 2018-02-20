#include "GameManagerSession.h"
#include "gamedefine.h"
#include "GameServer.h"

const static unsigned int g_dwGameManagerSessionBuffLen = 64 * 1024;
static char g_pGameManagerSessionBuf[g_dwGameManagerSessionBuffLen];

CGameManagerSession::CGameManagerSession()
{
}

CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
	stGAME_NOTIFY_GAME_MANAGER_INFO oGAME_NOTIFY_GAME_MANAGER_INFO;
	oGAME_NOTIFY_GAME_MANAGER_INFO.wPlayerPort = GameServer::Instance()->GetPlayerListenPort();
	oGAME_NOTIFY_GAME_MANAGER_INFO.wServerPort = GameServer::Instance()->GetServerListenPort();
	oGAME_NOTIFY_GAME_MANAGER_INFO.wSlaveServerPort = GameServer::Instance()->GetSlaveServerListenPort();
	CNetStream oStream(ENetStreamType_Write, g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen);
	oStream.WriteInt(Protocol::GAME_NOTIFY_GAME_MANAGER_INFO);

	oGAME_NOTIFY_GAME_MANAGER_INFO.Write(oStream);
	Send(g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen - oStream.GetDataLength());
}

void CGameManagerSession::OnClose(void)
{
}

void CGameManagerSession::OnError(UINT32 dwErrorNo)
{
}

void CGameManagerSession::OnRecv(const char * pBuf, UINT32 dwLen)
{
}

void CGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CGameManagerSession::OnGameNotifyGameManagerInfo(const char * pBuf, UINT32 dwLen)
{
}

//////////////////////////////////////////////////////////////////////////
CBinaryGameManagerSession::CBinaryGameManagerSession()
{
}

CBinaryGameManagerSession::~CBinaryGameManagerSession()
{
}

void CBinaryGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}
