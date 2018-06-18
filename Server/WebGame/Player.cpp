#include "Player.h"
#include "PlayerSession.h"


CPlayerBase::CPlayerBase()
	: m_pPlayerSession(NULL)
	, m_qwPlayerId(0)
{
}


CPlayerBase::~CPlayerBase()
{
}

//////////////////////////////////////////////////////////////////////////
CCommonPlayer::CCommonPlayer()
	: m_oTetrisData(*this)
{

}

CCommonPlayer::~CCommonPlayer() { }

void CCommonPlayer::Update(float fDelta)
{
	m_oTetrisData.Update(fDelta);
}

void CCommonPlayer::Init()
{
	m_oTetrisData.Init();
}

void CCommonPlayer::FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData)
{
	*(refRoleData.mutable_common_role_data()) = m_oGameRoleData;
}
