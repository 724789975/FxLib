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

bool GameServer::Init(std::string szServerListenIp, unsigned short wServerListenPort, std::string szPlayerListenIp, unsigned short wPlayerListenPort)
{
	m_oBinaryServerSessionManager.Init();
	m_oWebSocketPlayerSessionManager.Init();

	m_szServerListenIp = szServerListenIp;
	m_wServerListenPort = wServerListenPort;
	m_szPlayerListenIp = szPlayerListenIp;
	m_wPlayerListenPort = wPlayerListenPort;

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}

	m_pServerListenSocket = pNet->Listen(&GameServer::Instance()->GetBinaryServerSessionManager(), SLT_CommonTcp, inet_addr(m_szServerListenIp.c_str()), m_wServerListenPort);
	if (m_pServerListenSocket == NULL)
	{
		return false;
	}

	m_pPlayerListenSocket = pNet->Listen(&GameServer::Instance()->GetWebSocketPlayerSessionManager(), SLT_WebSocket, inet_addr(m_szPlayerListenIp.c_str()), m_wPlayerListenPort);
	if (m_pPlayerListenSocket == NULL)
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

bool GameServer::AddRequestPlayer(CPlayerSession* pPlayer)
{
	if (m_setRequestPlayer.find(pPlayer) == m_setRequestPlayer.end())
	{
		m_setRequestPlayer.insert(pPlayer);
		return true;
	}
	return false;
}

bool GameServer::DelRequestPlayer(CPlayerSession* pPlayer)
{
	if (m_setRequestPlayer.find(pPlayer) == m_setRequestPlayer.end())
	{
		return false;
	}
	m_setRequestPlayer.erase(pPlayer);
	return true;
}

