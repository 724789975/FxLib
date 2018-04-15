#include "Player.h"
#include "fxdb.h"
#include <map>
#include <string>
#include <sstream>
#include "gamedefine.h"
#include "TeamSession.h"
#include "GameServer.h"

const static unsigned int g_dwPlayerBuffLen = 64 * 1024;
static char g_pPlayerBuff[g_dwPlayerBuffLen];

Player::Player()
	: m_pSession(NULL)
	, m_qwPyayerId(0)
	, m_eState(PlayrState_Idle)
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

bool Player::OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginMakeTeam& refMsg)
{
	GameProto::LoginRequestTeamMakeTeam oTeam;
	oTeam.set_qw_player_id(m_qwPyayerId);

	GameProto::RoleData* pRoleData = oTeam.mutable_role_data();
	pRoleData->set_qw_player_id(m_qwPyayerId);
	pRoleData->set_sz_nick_name(m_szNickName);
	pRoleData->set_sz_avatar(m_szAvatar);
	pRoleData->set_dw_sex(m_dwSex);

	if (m_eState != PlayrState_Idle)
	{
		//只能在idle的情况下 才能创建队伍
		GameProto::LoginAckPlayerLoginResult oResult;
		oResult.set_dw_result(GameProto::EC_MakeTeamNotIdle);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return true;
	}
	std::map<unsigned int, CBinaryTeamSession*>& refSessions = GameServer::Instance()->GetTeamSessionManager().GetTeamSessions();
	if (refSessions.size() == 0)
	{
		GameProto::LoginAckPlayerLoginResult oResult;
		oResult.set_dw_result(GameProto::EC_NoTeamServer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		m_pSession->Send(pBuf, dwBufLen);
		return true;
	}

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oTeam, pBuf, dwBufLen);
	static unsigned int s_dwTeamIndex;
	unsigned int dwTeamIndex = ++s_dwTeamIndex % refSessions.size();
	std::map<unsigned int, CBinaryTeamSession*>::iterator it = refSessions.begin();
	for (int i = 0; i <= dwTeamIndex; ++i)
	{
		++it;
	}
	(it->second)->Send(pBuf, dwBufLen);
	return true;
}

bool Player::OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginInviteTeam& refMsg)
{
	return true;
}

bool Player::OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, GameProto::PlayerRequestLoginChangeSlot& refMsg)
{
	return true;
}
