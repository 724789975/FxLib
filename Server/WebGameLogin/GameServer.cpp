#include "GameServer.h"
#include "msg_proto/web_game.pb.h"


GameServer::GameServer()
	: m_pPlayerListenSocket(0)
	, m_wPlayerListenPort(0)
	, m_dwServerId(0)
	, m_wLoginPort(0)
	, m_wTeamPort(0)
	, m_wGameManagerPort(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init(unsigned int dwServerId, std::string szPlayerListenIp, unsigned short wPlayerListenPort, std::string szGameCenterIp, unsigned short wGameCenterPort, unsigned short wLoginPort, unsigned short wTeamPort, unsigned short wGameManagerPort)
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

	m_szGameCenterIp = szGameCenterIp;
	m_wGameCenterPort = wGameCenterPort;
	if (pNet->TcpConnect(&m_oCenterSession, 0, m_wGameCenterPort, false) == INVALID_SOCKET)
	{
		return false;
	}

	m_dwServerId = dwServerId;
	m_wLoginPort = wLoginPort;
	m_wTeamPort = wTeamPort;
	m_wGameManagerPort = wGameManagerPort;

	return true;
}

bool GameServer::Stop()
{
	// todo
	return false;
}

