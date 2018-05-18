#include "PlayerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "fxredis.h"

#ifdef WIN32
#include <Windows.h>
#include <ShellAPI.h>
#else
#include<unistd.h>
#endif // WIN32

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

class RedisGetServerId : public IRedisQuery
{
public:
	RedisGetServerId(UINT64 qwPlayerId) : m_qwPlayerId(qwPlayerId), m_dwServerId(0), m_pReader(NULL) {}
	~RedisGetServerId() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[64] = { 0 };
		sprintf(szQuery, "ZSCORE %s %llu", RedisConstant::szOnLinePlayer, m_qwPlayerId);
		poDBConnection->Query(szQuery, &m_pReader);
	}
	virtual void OnResult(void) { std::string szServerId; m_pReader->GetValue(szServerId); m_dwServerId = atoi(szServerId.c_str()); }
	virtual void Release(void) { m_pReader->Release(); }

	INT32 GetServerId() { return m_dwServerId; }

private:
	IRedisDataReader* m_pReader;
	INT32 m_dwServerId;
	UINT64 m_qwPlayerId;
};

CPlayerSession::CPlayerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginServerId::descriptor(), &CPlayerSession::OnPlayerRequestLoginServerId);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLogin::descriptor(), &CPlayerSession::OnPlayerRequestLogin);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginMakeTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginMakeTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginInviteTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginInviteTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginChangeSlot::descriptor(), &CPlayerSession::OnPlayerRequestLoginChangeSlot);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginGameStart::descriptor(), &CPlayerSession::OnPlayerRequestLoginGameStart);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginOnLinePlayer::descriptor(), &CPlayerSession::OnPlayerRequestLoginOnLinePlayer);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginEnterTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginEnterTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginRefuseEnterTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginRefuseEnterTeam);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLoginLeaveTeam::descriptor(), &CPlayerSession::OnPlayerRequestLoginLeaveTeam);
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
	oResult.set_dw_server_id(GameServer::Instance()->GetServerId());

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

	RedisGetServerId oServerId(pMsg->qw_player_id());
	FxRedisGetModule()->QueryDirect(&oServerId);
	UINT32 dwServerId = oServerId.GetServerId();

	GameServer::Instance()->GetPlayerManager().OnPlayerLogin(this, *pMsg);
	m_qwPlayerId = pMsg->qw_player_id();

	CLoginSession* pLoginSession = GameServer::Instance()->GetLoginSessionManager().GetLoginSession(dwServerId);
	if (pLoginSession)
	{
		GameProto::LoginNotifyLoginPlayerKick oKick;
		oKick.set_qw_player_id(pMsg->qw_player_id());
	}

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

bool CPlayerSession::OnPlayerRequestLoginOnLinePlayer(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginOnLinePlayer* pMsg = dynamic_cast<GameProto::PlayerRequestLoginOnLinePlayer*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	
	class RedisPlayerIds : public IRedisQuery
	{
	public:
		RedisPlayerIds() : m_pReader(NULL) {}
		~RedisPlayerIds() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "ZRANGE %s 0 -1", RedisConstant::szOnLinePlayer);
			poDBConnection->Query(szQuery, &m_pReader);
		}
		virtual void OnResult(void)
		{
			std::vector<std::string> vecIds;
			m_pReader->GetValue(vecIds);
			for (std::vector<std::string>::iterator it = vecIds.begin(); it != vecIds.end(); ++it)
			{
				m_vecPlayers.push_back(atoll(it->c_str()));
			}
		}
		virtual void Release(void) { m_pReader->Release(); }

		std::vector<UINT64>& GetPlayers() { return m_vecPlayers; }

	private:
		IRedisDataReader* m_pReader;
		std::vector<UINT64> m_vecPlayers;
	};

	RedisPlayerIds oServerPlayerIds;
	FxRedisGetModule()->QueryDirect(&oServerPlayerIds);

	GameProto::LoginAckPlayerOnLinePlayer oResult;
	std::vector<UINT64>& ref_vecPlayers = oServerPlayerIds.GetPlayers();
	for (std::vector<UINT64>::iterator it = ref_vecPlayers.begin(); it != ref_vecPlayers.end(); ++it)
	{
		oResult.add_qw_player_id(*it);
	}
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

bool CPlayerSession::OnPlayerRequestLoginEnterTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginEnterTeam* pMsg = dynamic_cast<GameProto::PlayerRequestLoginEnterTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer == NULL)
	{
		LogExe(LogLv_Critical, "cann't find player : %llu", m_qwPlayerId);
		return true;
	}

	CBinaryTeamSession* pTeamSession = GameServer::Instance()->GetTeamSessionManager().GetTeamSession(pMsg->dw_team_server_id());
	if (pTeamSession == NULL)
	{
		LogExe(LogLv_Critical, "no team server player : %llu, team server id : %d", m_qwPlayerId, pMsg->dw_team_server_id());
		GameProto::LoginAckPlayerEnterTeam oResult;
		oResult.set_dw_result(GameProto::EC_NoTeamServer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}

	GameProto::LoginRequestTeamEnterTeam oRequest;
	oRequest.set_qw_team_id(pMsg->qw_team_id());
	GameProto::RoleData* pRoleData = oRequest.mutable_role_data();
	pRoleData->set_qw_player_id(pPlayer->GetPlayerId());
	pRoleData->set_dw_sex(pPlayer->GetSex());
	pRoleData->set_sz_avatar(pPlayer->GetAvatar());
	pRoleData->set_sz_nick_name(pPlayer->GetNickName());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oRequest, pBuf, dwBufLen);
	pTeamSession->Send(pBuf, dwBufLen);

	return true;
}

bool CPlayerSession::OnPlayerRequestLoginRefuseEnterTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginRefuseEnterTeam* pMsg = dynamic_cast<GameProto::PlayerRequestLoginRefuseEnterTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer == NULL)
	{
		LogExe(LogLv_Critical, "cann't find player : %llu", m_qwPlayerId);
		Close();
		return true;
	}

	Player* pInvitePlay = GameServer::Instance()->GetPlayerManager().GetPlayer(pMsg->qw_player_id());
	if (pInvitePlay)
	{
		GameProto::LoginNotifyPlayerRefuseEnterTeam oNotify;
		oNotify.set_qw_player_id(m_qwPlayerId);
		oNotify.set_sz_reason(pMsg->sz_reason());

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
		pInvitePlay->GetSession()->Send(pBuf, dwBufLen);
		return true;
	}

	RedisGetServerId oServerId(pMsg->qw_player_id());
	FxRedisGetModule()->QueryDirect(&oServerId);
	UINT32 dwServerId = oServerId.GetServerId();

	CLoginSession* pLoginSession = GameServer::Instance()->GetLoginSessionManager().GetLoginSession(dwServerId);
	if (pLoginSession)
	{
		GameProto::LoginNotifyLoginPlayerRefuseEnterTeam oNotify;
		oNotify.set_qw_invite_id(pMsg->qw_player_id());
		oNotify.set_qw_invitee_id(m_qwPlayerId);
		oNotify.set_sz_reason(pMsg->sz_reason());

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
		pInvitePlay->GetSession()->Send(pBuf, dwBufLen);
		return true;
	}
}

bool CPlayerSession::OnPlayerRequestLoginLeaveTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLoginLeaveTeam* pMsg = dynamic_cast<GameProto::PlayerRequestLoginLeaveTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(m_qwPlayerId);
	if (pPlayer == NULL)
	{
		LogExe(LogLv_Critical, "cann't find player : %llu", m_qwPlayerId);
		Close();
		return true;
	}

	CBinaryTeamSession* pSession = GameServer::Instance()->GetTeamSessionManager().GetTeamSession(pPlayer->GetTeamServerId());
	if (!pSession)
	{
		LogExe(LogLv_Critical, "cann't find team session : %llu", pPlayer->GetTeamServerId());
		return true;
	}
	GameProto::LoginRequestTeamPlayerLeave oKickPlayer;
	oKickPlayer.set_qw_player_id(pPlayer->GetPlayerId());
	oKickPlayer.set_qw_team_id(pPlayer->GetTeamId());
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oKickPlayer, pBuf, dwBufLen);
	pSession->Send(pBuf, dwBufLen);
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

