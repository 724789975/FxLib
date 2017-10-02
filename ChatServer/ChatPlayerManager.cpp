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

ChatPlayer* ChatPlayerManager::OnPlayerLogin(std::string szPlayerId, std::string szSign, ChatSession* pSession)
{
	ChatPlayer* pPlayer = &m_mapPlayers[szPlayerId];
	pPlayer->Init(pSession, szPlayerId);

	return pPlayer;
}

void ChatPlayerManager::OnSessionClose(std::string szPlayerId)
{
	m_mapPlayers.erase(szPlayerId);
}

void ChatPlayerManager::OnBroadCastMsg(const Protocol::EChatType& eChatType, const std::string& szContent)
{
	//stCHAT_SEND_CHAT_PRIVATE_CHAT oCHAT_SEND_CHAT_PRIVATE_CHAT;
	//oCHAT_SEND_CHAT_PRIVATE_CHAT.szSenderId[0] = '0';
	//oCHAT_SEND_CHAT_PRIVATE_CHAT.eChatType = eChatType;
	//oCHAT_SEND_CHAT_PRIVATE_CHAT.szContent = szContent;

	for (std::map<std::string, ChatPlayer>::iterator it = m_mapPlayers.begin(); it != m_mapPlayers.end(); ++it)
	{
		it->second.OnChat("0", eChatType, szContent);
	}
}

std::string ChatPlayerManager::CreateLoginSign(std::string szPlayerId)
{
	std::string szSign = szPlayerId + "_sign";
	m_mapLoginSign[szPlayerId] = szSign;
	return szSign;
}
