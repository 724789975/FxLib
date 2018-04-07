#include "GameServer.h"



GameServer::GameServer()
	: m_pPlayerListenSocket(0)
	, m_wPlayerListenPort(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init( std::string szPlayerListenIp, unsigned short wPlayerListenPort)
{
	m_oWebSocketPlayerSessionManager.Init();

	m_szPlayerListenIp = szPlayerListenIp;
	m_wPlayerListenPort = wPlayerListenPort;

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}

	m_pPlayerListenSocket = pNet->Listen(&GameServer::Instance()->GetWebSocketPlayerSessionManager(), SLT_WebSocket, inet_addr(m_szPlayerListenIp.c_str()), m_wPlayerListenPort);
	if (m_pPlayerListenSocket == NULL)
	{
		return false;
	}

	if (!m_oPlayerManager.Init())
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

