#include "fxredis.h"

#include "GameConfigBase.h"
#include "gamedefine.h"
#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"


CGameConfigBase::CGameConfigBase()
{
}


CGameConfigBase::~CGameConfigBase()
{
}

bool CGameConfigBase::Init(unsigned int dwGameType)
{
	switch ((GameProto::EGameType)(dwGameType))
	{
	case GameProto::GT_Common:
	{
		if (CreateInstance(new CGameCommonConfig) == false)
		{
			return false;
		}
	}
		break;
	default:
		return false;
	}

	Instance()->SetGameType(dwGameType);

	return Instance()->Init();
}

//////////////////////////////////////////////////////////////////////////

bool CGameCommonConfig::Init()
{
	class RedisGetGameConfig : public IRedisQuery
	{
	public:
		RedisGetGameConfig() : m_dwPrepareTime(0), m_pReader(NULL) {}
		~RedisGetGameConfig() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "HMGET %s_%d %s %s %s %s",
				RedisConstant::szGameConfig, 1, RedisConstant::szGameType,
				RedisConstant::szPrepareTime, RedisConstant::szGameReadyTime, RedisConstant::szSuspendTime);
			poDBConnection->Query(szQuery, &m_pReader);
		}
		virtual void OnResult(void)
		{
			std::vector<std::string> vec_szServerId;
			m_pReader->GetValue(vec_szServerId);
			m_dwPrepareTime = atoi(vec_szServerId[1].c_str());
			m_dwGameReadyTime = atoi(vec_szServerId[2].c_str());
			m_fSuspendTime = atof(vec_szServerId[3].c_str());
		}
		virtual void Release(void) { m_pReader->Release(); }

		IRedisDataReader* m_pReader;
		UINT32 m_dwPrepareTime;
		UINT32 m_dwGameReadyTime;
		float m_fSuspendTime;
	};

	RedisGetGameConfig oConfig;
	FxRedisGetModule()->QueryDirect(&oConfig);

	m_oConfig.mutable_base_config()->set_dw_prepare_time(oConfig.m_dwPrepareTime);
	m_oConfig.mutable_base_config()->set_dw_game_ready_time(oConfig.m_dwGameReadyTime);
	m_oConfig.mutable_base_config()->set_f_suspend_time(oConfig.m_fSuspendTime);
	return true;
}

void CGameCommonConfig::SetGameType(unsigned int dwGameType)
{
	m_oConfig.mutable_base_config()->set_dw_game_type(dwGameType);
}

unsigned int CGameCommonConfig::GetGameType()
{
	return m_oConfig.mutable_base_config()->dw_game_type();
}

unsigned int CGameCommonConfig::GetPrepareTime()
{
	return m_oConfig.mutable_base_config()->dw_prepare_time();
}

unsigned int CGameCommonConfig::GetGameReadyTime()
{
	return m_oConfig.mutable_base_config()->dw_game_ready_time();
}

unsigned int CGameCommonConfig::GetSuspendTime()
{
	return m_oConfig.mutable_base_config()->f_suspend_time();
}

void CGameCommonConfig::FillProtoConfig(GameProto::GameNotifyPlayerGameConfig& refConfig)
{
	refConfig.set_dw_game_type(m_oConfig.mutable_base_config()->dw_game_type());
	*(refConfig.mutable_common_config()) = m_oConfig;
}
