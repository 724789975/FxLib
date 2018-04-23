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

	return true;
}

void Player::OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginMakeTeam& refMsg)
{
	GameProto::LoginAckPlayerMakeTeam oResult;
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

		INT64 GetTeamId() { return m_qwTeamId; }

	private:
		IRedisDataReader* m_pReader;
		INT64 m_qwTeamId;
	};

	RedisTeamId oTeamId;
	FxRedisGetModule()->QueryDirect(&oTeamId);
	oResult.set_qw_team_id(oTeamId.GetTeamId());

	//设置状态为请求组队
	m_eState = PlayrState_MakeTeam;
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	static unsigned int s_dwTeamIndex;
	unsigned int dwTeamIndex = ++s_dwTeamIndex % refSessions.size();
	std::map<unsigned int, CBinaryTeamSession*>::iterator it = refSessions.begin();
	for (int i = 0; i < dwTeamIndex; ++i)
	{
		++it;
	}
	(it->second)->Send(pBuf, dwBufLen);
	return;
}

bool Player::OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginInviteTeam& refMsg)
{
	return true;
}

bool Player::OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, GameProto::PlayerRequestLoginChangeSlot& refMsg)
{
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
		std::map<unsigned int, CBinaryTeamSession*>::iterator it =
			GameServer::Instance()->GetTeamSessionManager().GetTeamSessions().find(m_dwTeamServerId);
		if (it != GameServer::Instance()->GetTeamSessionManager().GetTeamSessions().end())
		{
			GameProto::LoginRequestTeamKickPlayer oKickPlayer;
			oKickPlayer.set_qw_player_id(m_qwPyayerId);
			oKickPlayer.set_qw_team_id(m_qwTeamId);
			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(oKickPlayer, pBuf, dwBufLen);
			it->second->Send(pBuf, dwBufLen);
		}
	}
}
