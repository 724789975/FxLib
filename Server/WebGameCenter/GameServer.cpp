#include "GameServer.h"



GameServer::GameServer()
	: m_pServerListenSocket(0)
	, m_wPlayerListenPort(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init( std::string szPlayerListenIp, unsigned short wPlayerListenPort)
{
	m_oBinaryServerSessionManager.Init();

	m_szPlayerListenIp = szPlayerListenIp;
	m_wPlayerListenPort = wPlayerListenPort;

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}

	m_pServerListenSocket = pNet->Listen(&GameServer::Instance()->GetBinaryServerSessionManager(), SLT_CommonTcp, inet_addr(m_szPlayerListenIp.c_str()), m_wPlayerListenPort);
	if (m_pServerListenSocket == NULL)
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

