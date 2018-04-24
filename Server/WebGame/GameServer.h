#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>

#include "singleton.h"
#include "ServerSession.h"
#include "GameManagerSession.h"
#include "SlaveServerSession.h"
#include "PlayerSession.h"

class GameServer : public TSingleton<GameServer>, public IFxTimer
{
public:
	GameServer();
	virtual ~GameServer();

	virtual bool OnTimer(double fSecond);

	bool Init(unsigned int dwGameManagerIp, unsigned short wGameManagerPort, unsigned long long qwTeamId);
	bool Stop();

	bool GameEnd();

	//CBinaryGameManagerSession& GetBinaryGameManagerSession() { return m_oBinaryGameManagerSession; }

	unsigned short GetPlayerListenPort() { return m_wPlayerListenPort; }
	unsigned short GetServerListenPort() { return m_wServerListenPort; }
	unsigned short GetSlaveServerListenPort() { return m_wSlaveServerListenPort; }
	unsigned long long GetPlayerPoint() { return m_qwTeamId; }
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
};


#endif // !__GameServer_H__
