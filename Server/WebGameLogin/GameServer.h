#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "ServerSession.h"
#include "PlayerSession.h"
#include "PlayerManager.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(std::string szPlayerListenIp, unsigned short wPlayerListenPort);
	bool Stop();

	WebSocketPlayerSessionManager& GetWebSocketPlayerSessionManager() { return m_oWebSocketPlayerSessionManager; }

	PlayerManager& GetPlayerManager() { return m_oPlayerManager; }

private:
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	IFxListenSocket* m_pPlayerListenSocket;

	PlayerManager m_oPlayerManager;

	std::string m_szPlayerListenIp;
	unsigned short m_wPlayerListenPort;
};


#endif // !__GameServer_H__
