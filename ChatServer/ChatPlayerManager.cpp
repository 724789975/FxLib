#include "ChatPlayerManager.h"



ChatPlayerManager::ChatPlayerManager()
{
}


ChatPlayerManager::~ChatPlayerManager()
{
}

ChatPlayer* ChatPlayerManager::GetChatPlayer(std::string szPlayerId)
{
	if (m_mapPlayers.find(szPlayerId) == m_mapPlayers.end())
	{
		return NULL;
	}
	return &m_mapPlayers[szPlayerId];
}

ChatPlayer* ChatPlayerManager::OnPlayerLogin(std::string szPlayerId, ChatSession* pSession)
{
	ChatPlayer* pPlayer = &m_mapPlayers[szPlayerId];
	pPlayer->Init(pSession, szPlayerId);

	return pPlayer;
}

void ChatPlayerManager::OnSessionClose(std::string szPlayerId)
{
	m_mapPlayers.erase(szPlayerId);
}
