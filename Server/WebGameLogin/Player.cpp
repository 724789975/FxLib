#include "Player.h"
#include "fxdb.h"
#include <map>
#include <string>
#include <sstream>
#include "gamedefine.h"
#include "TeamSession.h"
#include "GameServer.h"
#include "fxredis.h"

const static unsigned int g_dwPlayerBuffLen = 64 * 1024;
static char g_pPlayerBuff[g_dwPlayerBuffLen];

class RedisGetServerId : public IRedisQuery
{
public:
	RedisGetServerId(unsigned long long qwPlayerId) : m_qwPlayerId(qwPlayerId), m_dwServerId(0), m_pReader(NULL) {}
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

	int GetServerId() { return m_dwServerId; }

private:
	IRedisDataReader* m_pReader;
	int m_dwServerId;
	unsigned long long m_qwPlayerId;
};

Player::Player()
	: m_pSession(NULL)
	, m_qwPyayerId(0)
	, m_eState(PlayrState_Idle)
	, m_qwTeamId(0)
	, m_dwTeamServerId(0)
{
}


Player::~Player()
{
}

bool Player::OnPlayerRequestLogin(CPlayerSession& refSession, GameProto::PlayerRequestLogin& refLogin)
{
	m_pSession = &refSession;
	m_qwPyayerId = refLogin.qw_player_id();
	m_szNickName = refLogin.sz_nick_name();
	m_szAvatar = refLogin.sz_avatar();
	m_dwSex = refLogin.dw_sex();
	m_dwBalance = refLogin.dw_balance();
	m_szToken = refLogin.sz_token();

	class RedisServerId : public IRedisQuery
	{
	public:
		RedisServerId(unsigned long long qwPlayerId, unsigned int dwServerId) : m_qwPlayerId(qwPlayerId), m_dwServerId(dwServerId) {}
		~RedisServerId() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "ZADD %s %d %llu", RedisConstant::szOnLinePlayer, m_dwServerId, m_qwPlayerId);
			poDBConnection->Query(szQuery);
		}
		virtual void OnResult(void) { }
		virtual void Release(void) { }
	private:
		unsigned int m_dwServerId;
		unsigned long long m_qwPlayerId;
	};

	RedisServerId oServerId(refLogin.qw_player_id(), GameServer::Instance()->GetServerId());
	FxRedisGetModule()->QueryDirect(&oServerId);

	return true;
}

void Player::OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginMakeTeam& refMsg)
{
	GameProto::LoginAckPlayerMakeTeam oResult;
	if (m_qwTeamId != 0)
	{
		oResult.set_dw_result(GameProto::EC_AlreadyInTeam);
		oResult.set_qw_team_id(m_qwTeamId);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return;
	}
	if (m_eState != PlayrState_Idle)
	{
		//只能在idle的情况下 才能创建队伍
		oResult.set_dw_result(GameProto::EC_MakeTeamNotIdle);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return;
	}
	std::map<unsigned int, CBinaryTeamSession*>& refSessions = GameServer::Instance()->GetTeamSessionManager().GetTeamSessions();
	if (refSessions.size() == 0)
	{
		oResult.set_dw_result(GameProto::EC_NoTeamServer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return;
	}
	GameProto::LoginRequestTeamMakeTeam oTeam;

	LogExe(LogLv_Debug, "player : %llu want to make team", m_qwPyayerId);

	GameProto::RoleData* pRoleData = oTeam.mutable_role_data();
	pRoleData->set_qw_player_id(m_qwPyayerId);
	pRoleData->set_sz_nick_name(m_szNickName);
	pRoleData->set_sz_avatar(m_szAvatar);
	pRoleData->set_dw_sex(m_dwSex);

	class RedisTeamId : public IRedisQuery
	{
	public:
		RedisTeamId() : m_qwTeamId(0), m_pReader(NULL) {}
		~RedisTeamId() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "incr %s", RedisConstant::szTeamId);
			poDBConnection->Query(szQuery, &m_pReader);
		}
		virtual void OnResult(void) { m_pReader->GetValue(m_qwTeamId); }
		virtual void Release(void) { m_pReader->Release(); }

		long long GetTeamId() { return m_qwTeamId; }

	private:
		IRedisDataReader* m_pReader;
		long long m_qwTeamId;
	};

	RedisTeamId oTeamId;
	FxRedisGetModule()->QueryDirect(&oTeamId);
	oTeam.set_qw_team_id(oTeamId.GetTeamId());

	//设置状态为请求组队
	m_eState = PlayrState_MakeTeam;
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oTeam, pBuf, dwBufLen);
	static unsigned int s_dwTeamIndex;
	unsigned int dwTeamIndex = ++s_dwTeamIndex % refSessions.size();
	std::map<unsigned int, CBinaryTeamSession*>::iterator it = refSessions.begin();
	for (unsigned int i = 0; i < dwTeamIndex; ++i)
	{
		++it;
	}
	(it->second)->Send(pBuf, dwBufLen);
	return;
}

bool Player::OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginInviteTeam& refMsg)
{
	GameProto::LoginAckPlayerInviteTeam oResult;
	if (refMsg.qw_player_id() == m_qwPyayerId)
	{
		oResult.set_dw_result(GameProto::EC_AlreadyInTeam);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return true;
	}

	if (m_qwTeamId == 0)
	{
		oResult.set_dw_result(GameProto::EC_NoTeamId);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return true;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(refMsg.qw_player_id());
	if (pPlayer)
	{
		if (pPlayer->GetTeamId())
		{
			oResult.set_dw_result(GameProto::EC_AlreadyInTeam);
			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
			m_pSession->Send(pBuf, dwBufLen);
			return true;
		}
		GameProto::LoginNotifyPlayerInviteTeam oNotify;
		oNotify.set_qw_player_id(m_qwPyayerId);
		oNotify.set_qw_team_id(m_qwTeamId);
		oNotify.set_dw_team_server_id(m_dwTeamServerId);

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
		pPlayer->GetSession()->Send(pBuf, dwBufLen);

		char* pBuf1 = NULL;
		unsigned int dwBufLen1 = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf1, dwBufLen1);
		m_pSession->Send(pBuf1, dwBufLen1);

		return true;
	}

	RedisGetServerId oServerId(m_qwPyayerId);
	FxRedisGetModule()->QueryDirect(&oServerId);
	unsigned int dwServerId = oServerId.GetServerId();
	CLoginSession* pLoginSession = GameServer::Instance()->GetLoginSessionManager().GetLoginSession(dwServerId);

	if (pLoginSession)
	{
		GameProto::LoginRequestLoginInviteTeam oNotify;
		oNotify.set_qw_invite_id(m_qwPyayerId);
		oNotify.set_qw_invitee_id(refMsg.qw_player_id());
		oNotify.set_qw_team_id(m_qwTeamId);
		oNotify.set_dw_team_server_id(m_dwTeamServerId);

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
		pLoginSession->Send(pBuf, dwBufLen);

		return true;
	}

	oResult.set_dw_result(GameProto::EC_PlayerNotOnline);
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	m_pSession->Send(pBuf, dwBufLen);
	return true;
}

bool Player::OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, GameProto::PlayerRequestLoginChangeSlot& refMsg)
{
	CBinaryTeamSession* pTeamSession = GameServer::Instance()->GetTeamSessionManager().GetTeamSession(m_dwTeamServerId);
	if (pTeamSession == NULL)
	{
		LogExe(LogLv_Critical, "find team server error id : %d, team id : %llu", m_dwTeamServerId, m_qwTeamId);
		return true;
	}
	GameProto::LoginRequestTeamChangeSlot oRequest;
	oRequest.set_qw_player_id(m_qwPyayerId);
	oRequest.set_qw_team_id(m_qwTeamId);
	oRequest.set_dw_slot_id(refMsg.dw_slot_id());
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oRequest, pBuf, dwBufLen);
	LogExe(LogLv_Debug, "%s, team id : %llu, player id : %llu, team server id :%d",
		oRequest.GetTypeName().c_str(), m_qwTeamId, m_qwPyayerId, m_dwTeamServerId);
	pTeamSession->Send(pBuf, dwBufLen);
	return true;
}

bool Player::OnPlayerRequestLoginGameStart(CPlayerSession& refSession, GameProto::PlayerRequestLoginGameStart& refMsg)
{
	CBinaryTeamSession* pTeamSession = GameServer::Instance()->GetTeamSessionManager().GetTeamSession(m_dwTeamServerId);
	if (pTeamSession == NULL)
	{
		LogExe(LogLv_Critical, "find team server error id : %d, team id : %llu", m_dwTeamServerId, m_qwTeamId);
		return true;
	}

	GameProto::LoginRequestTeamGameStart oRequest;
	oRequest.set_qw_player_id(m_qwPyayerId);
	oRequest.set_qw_team_id(m_qwTeamId);
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oRequest, pBuf, dwBufLen);
	LogExe(LogLv_Debug, "%s, team id : %llu, player id : %llu, team server id :%d",
		oRequest.GetTypeName().c_str(), m_qwTeamId, m_qwPyayerId, m_dwTeamServerId);
	pTeamSession->Send(pBuf, dwBufLen);
	return true;
}

void Player::OnTeamKick()
{
	if (m_eState != PlayrState_MakeTeam && m_eState != PlayrState_TeamCompleted)
	{
		return;
	}
	m_eState = PlayrState_Idle;
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	GameProto::LoginNotifyPlayerTeamKick oResult;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	m_pSession->Send(pBuf, dwBufLen);

	m_dwTeamServerId = 0;
	m_qwTeamId = 0;
}

void Player::OnClose()
{
	if (m_qwTeamId)
	{
		CBinaryTeamSession* pSession = GameServer::Instance()->GetTeamSessionManager().GetTeamSession(m_dwTeamServerId);
		if (pSession)
		{
			GameProto::LoginRequestTeamKickPlayer oKickPlayer;
			oKickPlayer.set_qw_player_id(m_qwPyayerId);
			oKickPlayer.set_qw_team_id(m_qwTeamId);
			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(oKickPlayer, pBuf, dwBufLen);
			pSession->Send(pBuf, dwBufLen);
		}
	}

	RedisGetServerId oServerId(m_qwPyayerId);
	FxRedisGetModule()->QueryDirect(&oServerId);
	unsigned int dwServerId = oServerId.GetServerId();

	if (dwServerId == GameServer::Instance()->GetServerId())
	{
		class RedisSetPlayerOffLine : public IRedisQuery
		{
		public:
			RedisSetPlayerOffLine(unsigned long long qwPlayerId) : m_qwPlayerId(qwPlayerId), m_qwServerId(0) {}
			~RedisSetPlayerOffLine() {}

			virtual int					GetDBId(void) { return 0; }
			virtual void				OnQuery(IRedisConnection *poDBConnection)
			{
				char szQuery[64] = { 0 };
				sprintf(szQuery, "ZREM %s %llu", RedisConstant::szOnLinePlayer, m_qwPlayerId);
				poDBConnection->Query(szQuery);
			}
			virtual void OnResult(void) { }
			virtual void Release(void) { }

		private:
			long long m_qwServerId;
			unsigned long long m_qwPlayerId;
		};

		RedisSetPlayerOffLine oSetPlayerOffLine (m_qwPyayerId);
		FxRedisGetModule()->QueryDirect(&oSetPlayerOffLine );
	}
}
