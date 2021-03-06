#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>

#include "singleton.h"
#include "ServerSession.h"
#include "GameManagerSession.h"
#include "SlaveServerSession.h"
#include "PlayerSession.h"

#include "GameServerBase.h"

class GameServer : public TSingleton<GameServer>, public CGameServerBase
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(std::string szGameManagerIp, unsigned short wGameManagerPort, unsigned long long qwTeamId, unsigned int dwTeamServerId);
	bool Stop();

	bool GameEnd();

	//CBinaryGameManagerSession& GetBinaryGameManagerSession() { return m_oBinaryGameManagerSession; }

	unsigned short GetPlayerListenPort() { return m_wPlayerListenPort; }
	unsigned short GetServerListenPort() { return m_wServerListenPort; }
	unsigned short GetSlaveServerListenPort() { return m_wSlaveServerListenPort; }
	unsigned long long GetTeamId() { return m_qwTeamId; }
	unsigned int GetTeamServerId() { return m_dwTeamServerId; }
	WebSocketServerSessionManager& GetWebSocketServerSessionManager() { return m_oWebSocketServerSessionManager; }

private:
	CBinaryGameManagerSession m_oBinaryGameManagerSession;
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	WebSocketServerSessionManager m_oWebSocketServerSessionManager;
	WebSocketSlaveServerSessionManager m_oWebSocketSlaveServerSessionManager;

	IFxListenSocket* m_pPlayerListenSocket;
	unsigned short m_wPlayerListenPort;
	IFxListenSocket* m_pServerListenSocket;
	unsigned short m_wServerListenPort;
	IFxListenSocket* m_pSlaveServerListenSocket;
	unsigned short m_wSlaveServerListenPort;

	unsigned long long m_qwTeamId;
	unsigned int m_dwTeamServerId;
};


#endif // !__GameServer_H__
