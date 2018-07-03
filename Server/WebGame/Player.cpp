#include "Player.h"
#include "PlayerSession.h"
#include "fxredis.h"

class RedisGetPlayerData : public IRedisQuery
{
public:
	RedisGetPlayerData(UINT64 qwPlayerId) : m_qwPlayerId(qwPlayerId), m_pReader(NULL) {}
	~RedisGetPlayerData() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[128] = { 0 };
		sprintf(szQuery, "HMGET %llu_%s %s %s %s",
			m_qwPlayerId, RedisConstant::szRole, RedisConstant::szName,
			RedisConstant::szHeadImage, RedisConstant::szSex);
		poDBConnection->Query(szQuery, &m_pReader);
		LogExe(LogLv_Debug3, "redis query : %s", szQuery);
	}
	virtual void OnResult(void)
	{
		std::vector<std::string> vec_szServerId;
		m_pReader->GetValue(vec_szServerId);
		m_szName = vec_szServerId[0].c_str();
		m_szHeadImg = vec_szServerId[1].c_str();
		m_dwSex = atoi(vec_szServerId[2].c_str());
	}
	virtual void Release(void) { m_pReader->Release(); }

	std::string m_szName;
	std::string m_szHeadImg;
	UINT32 m_dwSex;
private:
	UINT64 m_qwPlayerId;
	IRedisDataReader* m_pReader;
};

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

void CCommonPlayer::FillPlayerData(GameProto::RoleData& refRoleData)
{
	refRoleData.CopyFrom(*m_oGameRoleData.mutable_role_data());
	LogExe(LogLv_Debug3, "role data : %s", refRoleData.DebugString().c_str());
}

void CCommonPlayer::GetPlayerData()
{
	RedisGetPlayerData oQuery(m_qwPlayerId);
	FxRedisGetModule()->QueryDirect(&oQuery);

	m_oGameRoleData.mutable_role_data()->set_qw_player_id(m_qwPlayerId);
	m_oGameRoleData.mutable_role_data()->set_sz_nick_name(oQuery.m_szName);
	m_oGameRoleData.mutable_role_data()->set_sz_avatar(oQuery.m_szHeadImg);
	m_oGameRoleData.mutable_role_data()->set_dw_sex(oQuery.m_dwSex);

	LogExe(LogLv_Debug3, "role data : %s", m_oGameRoleData.DebugString().c_str());
}
