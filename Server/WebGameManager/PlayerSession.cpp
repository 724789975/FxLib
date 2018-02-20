#include "PlayerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"

#ifdef WIN32
#include <Windows.h>
#include <ShellAPI.h>
#else
#include<unistd.h>
#endif // WIN32

bool StartProccess()
{
#ifdef WIN32
	SHELLEXECUTEINFO shell = { sizeof(shell) };
	shell.fMask = SEE_MASK_NOCLOSEPROCESS;
	shell.lpVerb = "open";
	shell.lpFile = "WebGame.exe";
	char szBuffer[512] = { 0 };
	GetExePath();
	sprintf(szBuffer, "--game_manager_ip %s --game_manager_port %d",
		GameServer::Instance()->GetServerListenIp().c_str(), GameServer::Instance()->GetServerListenPort());
	shell.lpParameters = szBuffer;
	shell.lpDirectory = GetExePath();
	shell.nShow = SW_SHOWNORMAL;
	BOOL ret = ShellExecuteEx(&shell);
	return ret == TRUE;
#else
	char szExePath[512] = { 0 };
	sprintf(szExePath, "%s/WebGame", GetExePath());
	char szServerPort[8];
	sprintf(szServerPort, "%d", GameServer::Instance()->GetServerListenPort());
	const char *arg[] = { szExePath, "--game_manager_ip", GameServer::Instance()->GetServerListenIp().c_str(),
		"--game_manager_port", szServerPort };
	int pid = vfork();

	if (pid == 0)
	{
		execv(arg[0], arg);
		_exit(0);
	}
	else
	{
		return true;
	}
	return false;
#endif // WIN32
}

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
	CNetStream oStream(pBuf, dwLen);
	Protocol::EGameProtocol eProrocol;
	oStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
	case Protocol::PLAYER_REQUEST_GAME_MANAGER_INFO:			OnRequestGameManagerInfo(pData, dwLen);	break;
	default:	Assert(0);	break;
	}
}

void CPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CPlayerSession::OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stPLAYER_REQUEST_GAME_MANAGER_INFO oPLAYER_REQUEST_GAME_MANAGER_INFO;
	oPLAYER_REQUEST_GAME_MANAGER_INFO.Read(oStream);

	if (StartProccess())
	{
		GameServer::Instance()->AddRequestPlayer(this);
	}
}

//////////////////////////////////////////////////////////////////////////
void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
FxSession * WebSocketPlayerSessionManager::CreateSession()
{
	FxSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool WebSocketPlayerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

