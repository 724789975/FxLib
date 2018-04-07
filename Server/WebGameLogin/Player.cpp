#include "Player.h"
#include "fxdb.h"
#include <string>
#include <sstream>

const static unsigned int g_dwChatPlayerBuffLen = 64 * 1024;
static char g_pChatPlayerBuff[g_dwChatPlayerBuffLen];

Player::Player()
	: m_pSession(NULL)
	, m_qwPyayerId(0)
{
}


Player::~Player()
{
}

bool Player::Init(CPlayerSession* pSession, GameProto::PlayerRequestLogin& refLogin)
{
	m_pSession = pSession;
	m_qwPyayerId = refLogin.qw_player_id();
	m_szNickName = refLogin.sz_nick_name();
	m_szAvatar = refLogin.sz_avatar();
	m_dwSex = refLogin.dw_sex();
	m_dwBalance = refLogin.dw_balance();
	m_szToken = refLogin.sz_token();

	return true;
}