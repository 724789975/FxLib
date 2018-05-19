#include "GameScene.h"
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
	return Instance()->Init();
}

void CGameSceneBase::GameEnd()
{
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i])
		{
			RedisDelPlayerTeamId oRedisDelPlayerTeamId(m_qwRoles[i]);
			FxRedisGetModule()->QueryDirect(&oRedisDelPlayerTeamId);
		}
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
	return true;
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

