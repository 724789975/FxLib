#include "GameServer.h"



GameServer::GameServer()
	: m_pServerListenSocket(0)
	, m_pPlayerListenSocket(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init(unsigned int dwServerId, std::string szCenterIp, unsigned short wCenterPort)
{
	m_dwServerId = dwServerId;
	m_szCenterIp = szCenterIp;
	m_wCenterPort = wCenterPort;
	
	if (!m_oBinaryLoginSessionManager.Init())
	{
		return false;
	}

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
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

