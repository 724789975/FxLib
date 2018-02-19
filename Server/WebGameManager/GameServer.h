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

	bool Init();
	bool Stop();

	void AddRequestPlayer(CPlayerSession* pPlayer);
	CPlayerSession* EndRequestPlayer();


	WebSocketPlayerSessionManager& GetWebSocketPlayerSessionManager() { return m_oWebSocketPlayerSessionManager; }
	BinaryServerSessionManager& GetBinaryServerSessionManager() { return m_oBinaryServerSessionManager; }

private:
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	BinaryServerSessionManager m_oBinaryServerSessionManager;

	std::list<CPlayerSession*> m_listRequestPlayer;
	std::set<CPlayerSession*> m_setRequestPlayer;
};


#endif // !__GameServer_H__
