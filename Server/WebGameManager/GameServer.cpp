#include "GameServer.h"



GameServer::GameServer()
	: m_pServerListenSocket(0)
	, m_pPlayerListenSocket(0)
	, m_wServerListenPort(0)
	, m_wPlayerListenPort(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init(unsigned int dwServerId, std::string szServerIp, std::string szCenterIp, unsigned short wCenterPort, unsigned short wServerListenPort, unsigned short wPlayerListenPort)
{
	m_dwServerId = dwServerId;
	m_szServerIp = szServerIp;
	m_szCenterIp = szCenterIp;
	m_wCenterPort = wCenterPort;
	
	m_wServerListenPort = wServerListenPort;
	m_wPlayerListenPort = wPlayerListenPort;

	if (!m_oBinaryServerSessionManager.Init())
	{
		return false;
	}
	if (!m_oWebSocketPlayerSessionManager.Init())
	{
		return false;
	}
	if (!m_oBinaryLoginSessionManager.Init())
	{
		return false;
	}
	if (!m_oBinaryTeamSessionManager.Init())
	{
		return false;
	}

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}

	m_pServerListenSocket = pNet->Listen(&m_oBinaryServerSessionManager, SLT_CommonTcp, 0, m_wServerListenPort);
	if (m_pServerListenSocket == NULL)
	{
		return false;
	}

	m_pPlayerListenSocket = pNet->Listen(&m_oWebSocketPlayerSessionManager, SLT_WebSocket, 0, m_wPlayerListenPort);
	if (m_pPlayerListenSocket == NULL)
	{
		return false;
	}

	if (pNet->TcpConnect(&m_oCenterSession, inet_addr(m_szCenterIp.c_str()), wCenterPort, false) == INVALID_SOCKET)
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

