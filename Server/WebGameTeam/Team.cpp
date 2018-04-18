#include "Team.h"
#include "fxredis.h"


CTeam::CTeam()
{
}

CTeam::~CTeam()
{
	//析构的时候 要把玩家的队伍id从redis中删掉
	class RedisTeamRelease : public IRedisQuery
	{
	public:
		RedisTeamRelease(UINT64 qwPlayerId) : m_pReader(NULL) { m_qwPlayerId = qwPlayerId; }
		~RedisTeamRelease() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "del %llu_%s", m_qwPlayerId,  RedisConstant::szTeamId);
			poDBConnection->Query(szQuery);
		}
		virtual void				OnResult(void) { }
		virtual void				Release(void) { }

	private:
		IRedisDataReader* m_pReader;
		UINT64 m_qwPlayerId;
	};

	for (std::map<UINT64, GameProto::TeamRoleData >::iterator it = m_mapRoles.begin();
		it != m_mapRoles.end(); ++it)
	{
		RedisTeamRelease oRelease(it->first);
		FxRedisGetModule()->QueryDirect(&oRelease);
	}
}

//////////////////////////////////////////////////////////////////////////
CTeamManager::CTeamManager()
{
}

CTeamManager::~CTeamManager()
{
}

CTeam& CTeamManager::CreateTeam()
{
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
		virtual void				OnResult(void)
		{
			m_pReader->GetValue(m_qwTeamId);
		}
		virtual void				Release(void)
		{
			m_pReader->Release();
		}

		UINT64 GetTeamId() { return m_qwTeamId; }

	private:
		IRedisDataReader* m_pReader;
		INT64 m_qwTeamId;
	};
	RedisTeamId oTeamId;
	FxRedisGetModule()->QueryDirect(&oTeamId);
	UINT64 qwTeamId = oTeamId.GetTeamId();
	if (m_mapTeams.find(qwTeamId) != m_mapTeams.end())
	{
		LogExe(LogLv_Critical, "already has team id : %llu", qwTeamId);
		Assert(0);
	}

	return m_mapTeams[qwTeamId];
}

bool CTeamManager::ReleaseTeam(UINT64 qwTeamId)
{
	std::map<UINT64, CTeam>::iterator it = m_mapTeams.find(qwTeamId);
	if (it == m_mapTeams.end())
	{
		return false;
	}
	m_mapTeams.erase(it);
	return true;
}
