#include "Team.h"
#include "LoginSession.h"
#include "GameServer.h"
#include "fxredis.h"
#include "msg_proto/web_game.pb.h"


CTeam::CTeam()
	: m_qwLeader(0)
	, m_eState(ETS_Idle)
{
	memset(m_pPlayerSlots, 0, sizeof(m_pPlayerSlots));
}

CTeam::~CTeam()
{
	//析构的时候 要把玩家的队伍id从redis中删掉
	class RedisTeamRelease : public IRedisQuery
	{
	public:
		RedisTeamRelease(unsigned long long qwPlayerId) : m_pReader(NULL) { m_qwPlayerId = qwPlayerId; }
		~RedisTeamRelease() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "DEL %llu_%s", m_qwPlayerId,  RedisConstant::szTeamId);
			poDBConnection->Query(szQuery);
		}
		virtual void				OnResult(void) { }
		virtual void				Release(void) { }

	private:
		IRedisDataReader* m_pReader;
		unsigned long long m_qwPlayerId;
	};

	for (std::map<unsigned long long, GameProto::TeamRoleData >::iterator it = m_mapPlayers.begin();
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
		if (m_pPlayerSlots[i] == 0)
		{
			m_pPlayerSlots[i] = refRoleData.qw_player_id();
			GameProto::TeamRoleData& refTeamRoleData = m_mapPlayers[refRoleData.qw_player_id()];
			refTeamRoleData.mutable_role_data()->CopyFrom(refRoleData);
			refTeamRoleData.set_dw_slot_id(i);
			bInsert = true;
			if (m_qwLeader == 0)
			{
				m_qwLeader = refRoleData.qw_player_id();
			}
			break;
		}
	}
	return bInsert;
}

bool CTeam::KickPlayer(unsigned long long qwPlayerId)
{
	if (m_mapPlayers.find(qwPlayerId) == m_mapPlayers.end())
	{
		return false;
	}
	m_pPlayerSlots[m_mapPlayers[qwPlayerId].dw_slot_id()] = 0;
	m_mapPlayers.erase(qwPlayerId);
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_pPlayerSlots[i] != 0)
		{
			m_qwLeader = m_pPlayerSlots[i];
			break;
		}
	}
	return true;
}

void CTeam::NotifyPlayer()
{
	GameProto::TeamNotifyLoginTeamInfo oNotify;
	oNotify.set_qw_team_id(m_qwTeamId);
	for (std::map<unsigned long long, GameProto::TeamRoleData >::iterator it = m_mapPlayers.begin();
		it != m_mapPlayers.end(); ++it)
	{
		GameProto::TeamRoleData* pTeamRoleData = oNotify.add_team_role_data();
		pTeamRoleData->CopyFrom(it->second);
	}

	for (std::map<unsigned long long, GameProto::TeamRoleData >::iterator it = m_mapPlayers.begin();
		it != m_mapPlayers.end(); ++it)
	{
		CBinaryLoginSession* pLoginSession = GameServer::Instance()->GetLoginSessionManager().GetLoginSession(it->second.dw_server_id());
		if (pLoginSession)
		{
			oNotify.set_qw_player_id(it->first);
			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
			pLoginSession->Send(pBuf, dwBufLen);
		}
	}
}

GameProto::EErrorCode CTeam::ChangeSlot(unsigned long long qwPlayerId, unsigned int dwSlotId)
{
	if (m_mapPlayers.find(qwPlayerId) == m_mapPlayers.end())
	{
		return GameProto::EC_CannotFindPlayer;
	}
	if (m_pPlayerSlots[dwSlotId])
	{
		return GameProto::EC_TeamSlotHasPlayer;
	}
	//换坑开始
	m_pPlayerSlots[dwSlotId] = qwPlayerId;
	m_pPlayerSlots[m_mapPlayers[qwPlayerId].dw_slot_id()] = 0;
	m_mapPlayers[qwPlayerId].set_dw_slot_id(dwSlotId);
	//换坑结束
	return GameProto::EC_NONE;
}

GameProto::TeamRoleData* CTeam::GetTeamRoleData(unsigned long long qwPlayerId)
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

CTeam* CTeamManager::GetTeam(unsigned long long qwTeamId)
{
	if (m_mapTeams.find(qwTeamId) == m_mapTeams.end())
	{
		return NULL;
	}
	return &m_mapTeams[qwTeamId];
}

CTeam& CTeamManager::CreateTeam(unsigned long long qwTeamId)
{
	if (m_mapTeams.find(qwTeamId) != m_mapTeams.end())
	{
		LogExe(LogLv_Critical, "already has team id : %llu", qwTeamId);
		Assert(0);
	}

	m_mapTeams[qwTeamId].SetTeamId(qwTeamId);
	return m_mapTeams[qwTeamId];
}

bool CTeamManager::ReleaseTeam(unsigned long long qwTeamId)
{
	std::map<unsigned long long, CTeam>::iterator it = m_mapTeams.find(qwTeamId);
	if (it == m_mapTeams.end())
	{
		return false;
	}
	m_mapTeams.erase(it);
	return true;
}
