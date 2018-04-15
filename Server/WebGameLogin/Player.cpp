#include "Player.h"
#include "fxdb.h"
#include <string>
#include <sstream>

const static unsigned int g_dwPlayerBuffLen = 64 * 1024;
static char g_pPlayerBuff[g_dwPlayerBuffLen];

Player::Player()
	: m_pSession(NULL)
	, m_qwPyayerId(0)
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
