#include "PlayerManager.h"
#include "gamedefine.h"


PlayerManager::PlayerManager()
{
}


PlayerManager::~PlayerManager()
{
}

Player* PlayerManager::GetPlayer(unsigned long long qwPlayerId)
{
	if (m_mapPlayers.find(qwPlayerId) == m_mapPlayers.end())
	{
		return NULL;
	}
	return &m_mapPlayers[qwPlayerId];
}

Player* PlayerManager::OnPlayerLogin(CPlayerSession* pSession, GameProto::PlayerRequestLogin& refLogin)
{
	Player* pPlayer = NULL;
	std::map<unsigned long long, Player>::iterator it = m_mapPlayers.find(refLogin.qw_player_id());
	if (it != m_mapPlayers.end())
	{
		GameProto::LoginNotifyPlayerGameKick oKick;
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oKick, pBuf, dwBufLen);
		it->second.GetSession()->Send(pBuf, dwBufLen);
		it->second.GetSession()->Close();
		pPlayer = &(it->second);
	}
	else
	{
		pPlayer = &m_mapPlayers[refLogin.qw_player_id()];
	}
	pPlayer->OnPlayerRequestLogin(*pSession, refLogin);

	return pPlayer;
}

void PlayerManager::OnSessionClose(unsigned long long qwPlayerId)
{
	m_mapPlayers.erase(qwPlayerId);
}
