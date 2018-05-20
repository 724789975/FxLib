#include "GameScene.h"
#include "GameConfigBase.h"
#include "../json/json.h"
#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"
#include "fxredis.h"

class RedisSetPlayerTeamId : public IRedisQuery
{
public:
	RedisSetPlayerTeamId(UINT64 qwTeamId, UINT64 qwPlayerId) : m_qwTeamId(qwTeamId), m_qwPlayerId(qwPlayerId) {}
	~RedisSetPlayerTeamId() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[64] = { 0 };
		sprintf(szQuery, "SET %llu_%s %llu", m_qwPlayerId, RedisConstant::szPlayerTeamId, m_qwTeamId);
		poDBConnection->Query(szQuery);
	}
	virtual void OnResult(void) { }
	virtual void Release(void) { }

private:
	UINT64 m_qwTeamId;
	UINT64 m_qwPlayerId;
};

class RedisDelPlayerTeamId : public IRedisQuery
{
public:
	RedisDelPlayerTeamId(UINT64 qwPlayerId) : m_qwPlayerId(qwPlayerId) {}
	~RedisDelPlayerTeamId() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[64] = { 0 };
		sprintf(szQuery, "DEL %llu_%s", m_qwPlayerId, RedisConstant::szPlayerTeamId);
		poDBConnection->Query(szQuery);
	}
	virtual void OnResult(void) { }
	virtual void Release(void) { }

private:
	UINT64 m_qwPlayerId;
};

CGameSceneBase::CGameSceneBase()
	: m_eGameSceneState(ESS_None)
{
}


CGameSceneBase::~CGameSceneBase()
{
}

bool CGameSceneBase::Init(unsigned int dwGameType, std::string szRoles, UINT64 qwTeamId)
{
	switch ((GameProto::EGameType)(dwGameType))
	{
		case GameProto::GT_Common:
		{
			if (CreateInstance(new CGameSceneCommon) == false)
			{
				return false;
			}
		}
		break;
		default:
			return false;
	}
	Json::Value jRoles;
	Json::Reader jReader;

	if (!jReader.parse(szRoles, jRoles))
	{
		return false;
	}
	if (!jRoles.isArray())
	{
		return false;
	}
	for (int i = 0; i < jRoles.size(); ++i)
	{
		Instance()->m_qwRoles[i] = jRoles[i].asInt64();
		if (Instance()->m_qwRoles[i])
		{
			RedisSetPlayerTeamId oRedisSetPlayerTeamId(qwTeamId, Instance()->m_qwRoles[i]);
			FxRedisGetModule()->QueryDirect(&oRedisSetPlayerTeamId);
		}
	}

	Instance()->m_qwTeamId = qwTeamId;
	Instance()->m_dwGameStartTime = GetTimeHandler()->GetSecond();
	return Instance()->Init();
}

void CGameSceneBase::Run(double fTime)
{
	switch (m_eGameSceneState)
	{
		case ESS_None: break;
		case ESS_Prepare: { Preparing(fTime); } break;
		case ESS_GameReady: { GameReady(fTime); } break;
		case ESS_Gaming: { Gaming(fTime); } break;
		case ESS_Transact: { Transacting(fTime); } break;
		default: {Assert(0); } break;
	}
}

void CGameSceneBase::Preparing(double fTime)
{
	static UINT32 s_dwNotifyTime = m_dwGameStartTime + 5;

	if (GetTimeHandler()->GetSecond() - m_dwGameStartTime > CGameConfigBase::Instance()->GetPrepareTime())
	{
		ChangeState(ESS_GameReady);
		return;
	}
	if (GetTimeHandler()->GetSecond() > s_dwNotifyTime)
	{
		// todo 广播

		//通知客户端频率 最后5秒 每秒一次 前面 5秒一次
		if (s_dwNotifyTime >= m_dwGameStartTime + CGameConfigBase::Instance()->GetPrepareTime() - 5)
		{
			s_dwNotifyTime += 1;
		}
		else
		{
			s_dwNotifyTime += 5;
		}
	}
}

void CGameSceneBase::GameReady(double fTime)
{

}

void CGameSceneBase::Gaming(double fTime)
{

}

void CGameSceneBase::Transacting(double fTime)
{

}

void CGameSceneBase::OnPrepare()
{
}

void CGameSceneBase::OnGameReady()
{
}

void CGameSceneBase::OnGameStart()
{
}

void CGameSceneBase::OnTransact()
{
}

void CGameSceneBase::GameEnd()
{
	LogExe(LogLv_Debug, "%s", "game end~~~~~");
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i])
		{
			RedisDelPlayerTeamId oRedisDelPlayerTeamId(m_qwRoles[i]);
			FxRedisGetModule()->QueryDirect(&oRedisDelPlayerTeamId);
		}
	}
}

void CGameSceneBase::ChangeState(EGameSceneState eGameSceneState)
{
	m_eGameSceneState = eGameSceneState;
	switch (m_eGameSceneState)
	{
		case ESS_Prepare: OnPrepare(); break;
		case ESS_GameReady: OnGameReady(); break;
		case ESS_Gaming: OnGameStart(); break;
		case ESS_Transact: OnTransact(); break;
		default:Assert(0); break;
	}
}

//////////////////////////////////////////////////////////////////////////
CGameSceneCommon::CGameSceneCommon()
{
}

CGameSceneCommon::~CGameSceneCommon()
{
}

bool CGameSceneCommon::Init()
{
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i] != 0)
		{
			m_mapPlayers[m_qwRoles[i]].SetPlayerSession(NULL);
		}
	}
	ChangeState(ESS_Prepare);
	return true;
}

void CGameSceneCommon::OnGameStart()
{

}

CPlayerBase* CGameSceneCommon::GetPlayer(UINT64 qwPlayerId)
{
	std::map<UINT64, CCommonPlayer>::iterator it = m_mapPlayers.find(qwPlayerId);
	if (it == m_mapPlayers.end())
	{
		return NULL;
	}
	return &(it->second);
}

