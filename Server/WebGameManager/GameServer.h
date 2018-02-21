#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>

#include "singleton.h"
#include "ServerSession.h"
#include "PlayerSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(std::string szServerListenIp, unsigned short wServerListenPort, std::string szPlayerListenIp, unsigned short wPlayerListenPort);
	bool Stop();

	bool AddRequestPlayer(CPlayerSession* pPlayer);
	bool DelRequestPlayer(CPlayerSession* pPlayer);

	WebSocketPlayerSessionManager& GetWebSocketPlayerSessionManager() { return m_oWebSocketPlayerSessionManager; }
	BinaryServerSessionManager& GetBinaryServerSessionManager() { return m_oBinaryServerSessionManager; }

	std::string GetServerListenIp() { return m_szServerListenIp; }
	unsigned short GetServerListenPort() { return m_wServerListenPort; }

private:
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	BinaryServerSessionManager m_oBinaryServerSessionManager;

	IFxListenSocket* m_pServerListenSocket;
	IFxListenSocket* m_pPlayerListenSocket;

	std::set<CPlayerSession*> m_setRequestPlayer;

	std::string m_szServerListenIp;
	unsigned short m_wServerListenPort;
	std::string m_szPlayerListenIp;
	unsigned short m_wPlayerListenPort;
};


#endif // !__GameServer_H__
