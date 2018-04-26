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
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginServerId::descriptor(), &CPlayerSession::OnPlayerRequestLoginServerId);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLogin::descriptor(), &CPlayerSession::OnPlayerRequestLogin);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginMakeTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginMakeTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginInviteTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginInviteTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginChangeSlot::descriptor(), &CPlayerSession::OnPlayerRequestLoginChangeSlot);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginGameStart::descriptor(), &CPlayerSession::OnPlayerRequestLoginGameStart);
}


CPlayerSession::~CPlayerSession()
{
}

void CPlayerSession::OnConnect(void)
{
}

void CPlayerSession::OnClose(void)
{
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer)
	{
		pPlayer->OnClose();
	}
	else
	{
		LogExe(LogLv_Critical, "can't find player %llu", m_qwPlayerId);
		Close();
	}
}

void CPlayerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CPlayerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	std::string szProtocolName;
	oStream.ReadString(szProtocolName);
	LogExe(LogLv_Debug, "player : %llu, protocol name : %s", m_qwPlayerId, szProtocolName.c_str());
	unsigned int dwProtoLen = oStream.GetDataLength();
	char* pData = oStream.ReadData(dwProtoLen);
	if (!m_oProtoDispatch.Dispatch(szProtocolName.c_str(),
		(const unsigned char*)pData, dwProtoLen, this, *this))
	{
		LogExe(LogLv_Debug, "%s proccess error", szProtocolName.c_str());
	}
}

void CPlayerSession::Init()
{
	m_qwPlayerId = 0;
}

bool CPlayerSession::OnPlayerRequestLoginServerId(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginServerId* pMsg = dynamic_cast<GameProto::PlayerRequestLoginServerId*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	GameProto::LoginAckPlayerServerId oResult;
	oResult.set_dw_result(0);
	oResult.set_dw_server_id(GameServer::Instance()->GetServerid());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

bool CPlayerSession::OnPlayerRequestLogin(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLogin* pMsg = dynamic_cast<GameProto::PlayerRequestLogin*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameServer::Instance()->GetPlayerManager().OnPlayerLogin(this, *pMsg);
	m_qwPlayerId = pMsg->qw_player_id();

	GameProto::LoginAckPlayerLoginResult oResult;
	oResult.set_dw_result(0);

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

bool CPlayerSession::OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginMakeTeam* pMsg = dynamic_cast<GameProto::PlayerRequestLoginMakeTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	Player* pPlayer =  GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer)
	{
		pPlayer->OnPlayerRequestLoginMakeTeam(*this, *pMsg);
	}
	else
	{
		LogExe(LogLv_Critical, "can't find player %llu", m_qwPlayerId);
		Close();
	}
	return true;
}

bool CPlayerSession::OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginInviteTeam* pMsg = dynamic_cast<GameProto::PlayerRequestLoginInviteTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer)
	{
		return pPlayer->OnPlayerRequestLoginInviteTeam(*this, *pMsg);
	}
	else
	{
		LogExe(LogLv_Critical, "can't find player %llu", m_qwPlayerId);
	}
	return true;
}

bool CPlayerSession::OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginChangeSlot* pMsg = dynamic_cast<GameProto::PlayerRequestLoginChangeSlot*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer)
	{
		return pPlayer->OnPlayerRequestLoginChangeSlot(*this, *pMsg);
	}
	else
	{
		LogExe(LogLv_Critical, "can't find player %llu", m_qwPlayerId);
	}
	return true;
}

bool CPlayerSession::OnPlayerRequestLoginGameStart(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginGameStart* pMsg = dynamic_cast<GameProto::PlayerRequestLoginGameStart*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer)
	{
		return pPlayer->OnPlayerRequestLoginGameStart(*this, *pMsg);
	}
	else
	{
		LogExe(LogLv_Critical, "can't find player %llu", m_qwPlayerId);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());

	GameServer::Instance()->GetPlayerManager().OnSessionClose(m_qwPlayerId);
	OnDestroy();

	GameServer::Instance()->GetPlayerSessionManager().Release(this);
}

//////////////////////////////////////////////////////////////////////////
CWebSocketPlayerSession * WebSocketPlayerSessionManager::CreateSession()
{
	CWebSocketPlayerSession* pSession = m_poolSessions.FetchObj();
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

