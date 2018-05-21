#include "Player.h"
#include "PlayerSession.h"


CPlayerBase::CPlayerBase()
	: m_pPlayerSession(NULL)
{
}


CPlayerBase::~CPlayerBase()
{
}

//////////////////////////////////////////////////////////////////////////
CCommonPlayer::CCommonPlayer()
{

}

CCommonPlayer::~CCommonPlayer()
{

}

void CCommonPlayer::FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData)
{
	*(refRoleData.mutable_common_role_data()) = m_oGameRoleData;
}
