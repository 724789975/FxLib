#include "GameSession.h"
#include "netstream.h"
#include "chatdefine.h"
#include "ChatServerManager.h"

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

void GameSession::OnError(UINT32 dwErrorNo)
{

}

void GameSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oNetStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::GAME_REQUEST_CHAT_MANAGER_LOGIN:	OnLogin(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocol : %d", (unsigned int)eProrocol); Close(); }	break;
	}
}

void GameSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	m_szServerId = "";
	OnDestroy();

	Init(NULL);

	ChatServerManager::Instance()->GetGameSessionManager().Release(this);
}

void GameSession::OnLogin(const char* pBuf, UINT32 dwLen)
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

	CNetStream oStreamGame(ENetStreamType_Write, g_pGameSessionBuf, g_dwGameSessionBuffLen);
	oStreamGame.WriteInt(Protocol::CHAT_MANAGER_ACK_GAME_LOGIN);
	oCHAT_MANAGER_ACK_GAME_LOGIN.Write(oStreamGame);
	Send(g_pGameSessionBuf, g_dwGameSessionBuffLen - oStreamGame.GetDataLength());
}

FxSession* GameSessionManager::CreateSession()
{
	m_oLock.Lock();
	GameSession* pSession = m_poolSessions.FetchObj();
	m_oLock.UnLock();
	return pSession;
}

bool GameSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void GameSessionManager::CloseSessions()
{

}

void GameSessionManager::Release(FxSession* pSession)
{
	Assert(0);
}

void GameSessionManager::Release(GameSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}
