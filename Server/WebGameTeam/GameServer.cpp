#include "GameServer.h"



GameServer::GameServer()
	: m_pGameManagerListenSocket(0)
	, m_dwServerId(0)
	, m_wCenterPort(0)
	, m_wGameManagerPort(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init(unsigned int dwServerId, std::string szCenterIp, unsigned short wCenterPort, unsigned short wGameManagerPort)
{
	m_dwServerId = dwServerId;
	m_szCenterIp = szCenterIp;
	m_wCenterPort = wCenterPort;
	m_wGameManagerPort = wGameManagerPort;

	if (!m_oBinaryLoginSessionManager.Init())
	{
		return false;
	}

	if (!m_oBinaryGameManagerSessionManager.Init())
	{
		return false;
	}

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}

	m_pGameManagerListenSocket = pNet->Listen(&m_oBinaryGameManagerSessionManager, SLT_CommonTcp, 0, m_wGameManagerPort);
	if (m_pGameManagerListenSocket == NULL)
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

