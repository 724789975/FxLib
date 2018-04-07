#ifndef __ChatPlayerManager_H__
#define __ChatPlayerManager_H__

#include <map>
#include "Player.h"

class PlayerManager
{
public:
	PlayerManager();
	virtual ~PlayerManager();

	bool Init() { return true; }

	Player* GetPlayer(UINT64 qwPlayerId);
	std::map<UINT64, Player>& GetPlayerMap() { return m_mapPlayers; }

	Player* OnPlayerLogin(CPlayerSession* pSession, GameProto::PlayerRequestLogin& refLogin);
	void OnSessionClose(UINT64 qwPlayerId);

private:
	std::map<UINT64, Player> m_mapPlayers;
};

#endif // __ChatPlayerManager_H__
