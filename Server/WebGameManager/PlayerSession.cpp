#include "PlayerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"

#ifdef WIN32
#include <Windows.h>
#include <ShellAPI.h>
#else
#include<unistd.h>
#endif // WIN32

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

CPlayerSession::CPlayerSession()
{
}


CPlayerSession::~CPlayerSession()
{
}

void CPlayerSession::OnConnect(void)
{

}

void CPlayerSession::OnClose(void)
{

}

void CPlayerSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CPlayerSession::OnRecv(const char* pBuf, unsigned int dwLen)
{
	//CNetStream oStream(pBuf, dwLen);
	//Protocol::EGameProtocol eProrocol;
	//oStream.ReadInt((int&)eProrocol);
	//const char* pData = pBuf + sizeof(UINT32);
	//dwLen -= sizeof(UINT32);

	//switch (eProrocol)
	//{
	//case Protocol::PLAYER_REQUEST_GAME_MANAGER_INFO:			OnRequestGameManagerInfo(pData, dwLen);	break;
	//default:	Assert(0);	break;
	//}
}

void CPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CPlayerSession::OnRequestGameManagerInfo(const char* pBuf, unsigned int dwLen)
{
	//CNetStream oStream(pBuf, dwLen);
	//stPLAYER_REQUEST_GAME_MANAGER_INFO oPLAYER_REQUEST_GAME_MANAGER_INFO;
	//oPLAYER_REQUEST_GAME_MANAGER_INFO.Read(oStream);

	//if (StartProccess((unsigned long long)this))
	//{
	//	GameServer::Instance()->AddRequestPlayer(this);
	//}
}

void CPlayerSession::OnGameInfo(GameProto::GameNotifyGameManagerInfo& refInfo)
{
	//CNetStream oStream(ENetStreamType_Write, g_pPlayerSessionBuf, g_dwPlayerSessionBuffLen);
	//oStream.WriteInt(Protocol::GAME_MANAGER_ACK_PLAYER_INFO_RESULT);
	//stGAME_MANAGER_ACK_PLAYER_INFO_RESULT oGAME_MANAGER_ACK_PLAYER_INFO_RESULT;
	//oGAME_MANAGER_ACK_PLAYER_INFO_RESULT.wPlayerPort = refInfo.dw_player_port();
	//oGAME_MANAGER_ACK_PLAYER_INFO_RESULT.wServerPort = refInfo.dw_server_port();
	//oGAME_MANAGER_ACK_PLAYER_INFO_RESULT.wSlaveServerPort = refInfo.dw_slave_server_port();

	//oGAME_MANAGER_ACK_PLAYER_INFO_RESULT.Write(oStream);
	//Send(g_pPlayerSessionBuf, g_dwPlayerSessionBuffLen - oStream.GetDataLength());
}

//////////////////////////////////////////////////////////////////////////
void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
FxSession * WebSocketPlayerSessionManager::CreateSession()
{
	FxSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool WebSocketPlayerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

