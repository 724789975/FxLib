#ifndef __ChatPlayerManager_H__
#define __ChatPlayerManager_H__

#include <map>
#include "ChatPlayer.h"

class ChatPlayerManager
{
public:
	ChatPlayerManager();
	virtual ~ChatPlayerManager();

	bool Init() { return true; }

	ChatPlayer* GetChatPlayer(std::string szPlayerId);
	std::map<std::string, ChatPlayer>& GetPlayerMap() { return m_mapPlayers; }

	ChatPlayer* OnPlayerLogin(std::string szPlayerId, ChatSession* pSession);
	void OnSessionClose(std::string szPlayerId);

private:
	std::map<std::string, ChatPlayer> m_mapPlayers;
};

#endif // __ChatPlayerManager_H__
