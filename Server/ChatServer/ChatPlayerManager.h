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

	ChatPlayer* OnPlayerLogin(std::string szPlayerId, std::string szSign, ChatSession* pSession);
	void OnSessionClose(std::string szPlayerId);

	void OnBroadCastMsg(const Protocol::EChatType& eChatType, const std::string& szContent);

	std::string CreateLoginSign(std::string szPlayerId);
private:
	std::map<std::string, ChatPlayer> m_mapPlayers;

	std::map<std::string, std::string> m_mapLoginSign;
};

#endif // __ChatPlayerManager_H__
