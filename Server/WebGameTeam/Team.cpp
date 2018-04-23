#include "Team.h"
#include "fxredis.h"


CTeam::CTeam()
	: m_qwLeader(0)
{
	memset(m_oPlayerSlots, 0, MAXCLIENTNUM);
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

	for (std::map<UINT64, GameProto::TeamRoleData >::iterator it = m_mapPlayers.begin();
		it != m_mapPlayers.end(); ++it)
	{
		RedisTeamRelease oRelease(it->first);
		FxRedisGetModule()->QueryDirect(&oRelease);
	}
}

bool CTeam::InsertIntoTeam(const GameProto::RoleData& refRoleData)
{
	if (m_mapPlayers.size() >= MAXCLIENTNUM)
	{
		LogExe(LogLv_Critical, "players has enough");
		return false;
	}
	if (m_mapPlayers.find(refRoleData.qw_player_id()) != m_mapPlayers.end())
	{
		LogExe(LogLv_Critical, "players : %llu already in team", refRoleData.qw_player_id());
		return false;
	}
	bool bInsert = false;
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_oPlayerSlots[i] == 0)
		{
			m_oPlayerSlots[i] = refRoleData.qw_player_id();
			GameProto::TeamRoleData& refTeamRoleData = m_mapPlayers[refRoleData.qw_player_id()];
			refTeamRoleData.mutable_role_data()->CopyFrom(refRoleData);
			refTeamRoleData.set_dw_slot_id(i);
			bInsert = true;
			break;
		}
	}
	return bInsert;
}

bool CTeam::KickPlayer(UINT64 qwPlayerId)
{
	if (m_mapPlayers.find(qwPlayerId) == m_mapPlayers.end())
	{
		return false;
	}
	m_oPlayerSlots[m_mapPlayers[qwPlayerId].dw_slot_id()] = 0;
	m_mapPlayers.erase(qwPlayerId);
	return true;
}

GameProto::TeamRoleData* CTeam::GetTeamRoleData(UINT64 qwPlayerId)
{
	if (m_mapPlayers.find(qwPlayerId) == m_mapPlayers.end())
	{
		return NULL;
	}
	return &m_mapPlayers[qwPlayerId];
}

//////////////////////////////////////////////////////////////////////////
CTeamManager::CTeamManager()
{
}

CTeamManager::~CTeamManager()
{
}

CTeam* CTeamManager::GetTeam(UINT64 qwTeamId)
{
	if (m_mapTeams.find(qwTeamId) == m_mapTeams.end())
	{
		return NULL;
	}
	return &m_mapTeams[qwTeamId];
}

CTeam& CTeamManager::CreateTeam(UINT64 qwTeamId)
{
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
