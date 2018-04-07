#include "PlayerManager.h"



PlayerManager::PlayerManager()
{
}


PlayerManager::~PlayerManager()
{
}

Player* PlayerManager::GetPlayer(UINT64 qwPlayerId)
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
	pPlayer = &m_mapPlayers[refLogin.qw_player_id()];
	pPlayer->Init(pSession, refLogin);

	return pPlayer;
}

void PlayerManager::OnSessionClose(UINT64 qwPlayerId)
{
	m_mapPlayers.erase(qwPlayerId);
}
