#include "GameSession.h"
#include "netstream.h"
#include "chatdefine.h"
#include "ChatServerManager.h"
#include "utility.h"

const static unsigned int g_dwGameSessionBuffLen = 64 * 1024;
static char g_pGameSessionBuf[g_dwGameSessionBuffLen];

GameSession::GameSession()
{
}

GameSession::~GameSession()
{
}

void GameSession::OnConnect(void)
{

}

void GameSession::OnClose(void)
{

}

void GameSession::OnError(unsigned int dwErrorNo)
{

}

void GameSession::OnRecv(const char* pBuf, unsigned int dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oNetStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(unsigned int);
	dwLen -= sizeof(unsigned int);

	switch (eProrocol)
	{
		case Protocol::GAME_REQUEST_CHAT_MANAGER_LOGIN:	OnLogin(pData, dwLen);	break;
		case Protocol::GAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN:	OnPlayerLogin(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocol : %d", (unsigned int)eProrocol); Close(); }	break;
	}
}

void GameSession::Release(void)
{
	std::string szServerId = m_szServerId;
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	m_szServerId = "";
	OnDestroy();

	Init(NULL);

	ChatServerManager::Instance()->GetGameSessionManager().Release(szServerId);
}

void GameSession::OnLoginSign(std::string szChatIp, unsigned int dwChatPort, unsigned int dwWebSocketChatPort, std::string szPlayerId, std::string szSign)
{
	stCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN;
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.szListenIp = szChatIp;
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.szPlayerId = szPlayerId;
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.szSign = szSign;
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.dwTcpPort = dwChatPort;
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.dwWebSocketPort = dwWebSocketChatPort;

	CNetStream oStreamGame(ENetStreamType_Write, g_pGameSessionBuf, g_dwGameSessionBuffLen);
	oStreamGame.WriteInt(Protocol::CHAT_MANAGER_ACK_GAME_PLAYER_LOGIN);
	oCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN.Write(oStreamGame);
	Send(g_pGameSessionBuf, g_dwGameSessionBuffLen - oStreamGame.GetDataLength());
}

void GameSession::OnLogin(const char* pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stGAME_REQUEST_CHAT_MANAGER_LOGIN oGAME_REQUEST_CHAT_MANAGER_LOGIN;
	if (!oGAME_REQUEST_CHAT_MANAGER_LOGIN.Read(oStream))
	{
		LogExe(LogLv_Critical, "error");
		return;
	}

	stCHAT_MANAGER_ACK_GAME_LOGIN oCHAT_MANAGER_ACK_GAME_LOGIN;
	oCHAT_MANAGER_ACK_GAME_LOGIN.dwResult = 0;
	if (m_szServerId.size())
	{
		LogExe(LogLv_Critical, "already init server id : %s, new id : %s", m_szServerId.c_str(), oGAME_REQUEST_CHAT_MANAGER_LOGIN.szId.c_str());
		oCHAT_MANAGER_ACK_GAME_LOGIN.dwResult = 1;
		return;
	}

	m_szServerId = oGAME_REQUEST_CHAT_MANAGER_LOGIN.szId;

	ChatServerManager::Instance()->GetGameSessionManager().SetGameSession(m_szServerId, this);

	CNetStream oStreamGame(ENetStreamType_Write, g_pGameSessionBuf, g_dwGameSessionBuffLen);
	oStreamGame.WriteInt(Protocol::CHAT_MANAGER_ACK_GAME_LOGIN);
	oCHAT_MANAGER_ACK_GAME_LOGIN.Write(oStreamGame);
	Send(g_pGameSessionBuf, g_dwGameSessionBuffLen - oStreamGame.GetDataLength());
}

void GameSession::OnPlayerLogin(const char * pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN oGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN;
	if (!oGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN.Read(oStream))
	{
		LogExe(LogLv_Critical, "error");
		return;
	}

	unsigned int dwHashIndex = HashToIndex(oGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN.szPlayerId.c_str(), oGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN.szPlayerId.size());
	ChatServerSession* pServerSession = ChatServerManager::Instance()->GetChatServerSessionManager().GetChatServerSession(dwHashIndex);
	if (!pServerSession)
	{
		LogExe(LogLv_Critical, "error");
		return;
	}

	pServerSession->ChatLogin(m_szServerId, oGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN.szPlayerId);
}

FxSession* GameSessionManager::CreateSession()
{
	GameSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool GameSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void GameSessionManager::CloseSessions()
{

}

void GameSessionManager::SetGameSession(std::string szServerId, GameSession* pSession)
{
	m_mapGameSessions[szServerId] = pSession;
}

GameSession* GameSessionManager::GetGameSession(std::string szServerId)
{
	if (m_mapGameSessions.find(szServerId) == m_mapGameSessions.end())
	{
		return NULL;
	}
	return m_mapGameSessions[szServerId];
}

void GameSessionManager::Release(FxSession* pSession)
{
	Assert(0);
}

void GameSessionManager::Release(std::string szServerId)
{
	if (m_mapGameSessions.find(szServerId) == m_mapGameSessions.end())
	{
		LogExe(LogLv_Critical, "error");
		return;
	}
	m_poolSessions.ReleaseObj(m_mapGameSessions[szServerId]);
	m_mapGameSessions.erase(szServerId);
}
