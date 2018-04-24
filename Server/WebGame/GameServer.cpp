#include "GameServer.h"



GameServer::GameServer()
	: m_pServerListenSocket(NULL)
	, m_pPlayerListenSocket(NULL)
	, m_pSlaveServerListenSocket(NULL)
	, m_wPlayerListenPort(0)
	, m_wServerListenPort(0)
	, m_wSlaveServerListenPort(0)
	, m_qwTeamId(0)
	, m_dwTeamServerId(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::OnTimer(double fSecond)
{
	LogExe(LogLv_Debug, "%s", "time end~~~~~");
	GameEnd();
	exit(0);
}

bool GameServer::Init(unsigned int dwGameManagerIp, unsigned short wGameManagerPort, unsigned long long qwTeamId, unsigned int dwTeamServerId)
{
	m_qwTeamId = qwTeamId;
	m_dwTeamServerId = dwTeamServerId;
	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}
	m_pPlayerListenSocket = pNet->Listen(&m_oWebSocketPlayerSessionManager, SLT_WebSocket, 0, m_wPlayerListenPort);
	if (m_pPlayerListenSocket == NULL)
	{
		return false;
	}
	m_pServerListenSocket = pNet->Listen(&m_oWebSocketServerSessionManager, SLT_WebSocket, 0, m_wServerListenPort);
	if (m_pServerListenSocket == NULL)
	{
		return false;
	}
	m_pSlaveServerListenSocket = pNet->Listen(&m_oWebSocketSlaveServerSessionManager, SLT_WebSocket, 0, m_wSlaveServerListenPort);
	if (m_pSlaveServerListenSocket == NULL)
	{
		return false;
	}

	if (pNet->TcpConnect(&m_oBinaryGameManagerSession, dwGameManagerIp, wGameManagerPort, true) == INVALID_SOCKET)
	{
		return false;
	}
	return true;
}

bool GameServer::Stop()
{
	// todo
	return false;
}

bool GameServer::GameEnd()
{
	LogExe(LogLv_Debug, "%s", "game end~~~~~");
	return false;
}
