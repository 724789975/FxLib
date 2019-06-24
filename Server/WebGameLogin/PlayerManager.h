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

	Player* GetPlayer(unsigned long long qwPlayerId);
	std::map<unsigned long long, Player>& GetPlayerMap() { return m_mapPlayers; }

	Player* OnPlayerLogin(CPlayerSession* pSession, GameProto::PlayerRequestLogin& refLogin);
	void OnSessionClose(unsigned long long qwPlayerId);

private:
	std::map<unsigned long long, Player> m_mapPlayers;
};

#endif // __ChatPlayerManager_H__
