#include "GameServer.h"



GameServer::GameServer()
{
}


GameServer::~GameServer()
{
}

bool GameServer::Init()
{
	m_oBinaryServerSessionManager.Init();
	m_oWebSocketPlayerSessionManager.Init();
	return true;
}

bool GameServer::Stop()
{
	// todo
	return false;
}

void GameServer::AddRequestPlayer(CPlayerSession * pPlayer)
{
	if (m_setRequestPlayer.find(pPlayer) == m_setRequestPlayer.end())
	{
		m_setRequestPlayer.insert(pPlayer);
	}
}

CPlayerSession * GameServer::EndRequestPlayer()
{
	CPlayerSession* pPlayer = NULL;
	if (m_listRequestPlayer.size())
	{
		pPlayer = *(m_listRequestPlayer.begin());
		m_listRequestPlayer.pop_front();
		m_setRequestPlayer.erase(pPlayer);
	}
	return pPlayer;
}

