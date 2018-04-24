#ifndef __Team_H__
#define __Team_H__
#include "msg_proto/web_data.pb.h"
#include "gamedefine.h"
#include <map>

//后面可以加其他模式
class CTeam
{
public:
	enum ETeamState
	{
		ETS_NONE,
		ETS_Idle,
		ETS_StartGame,
	};
	CTeam();
	~CTeam();

	UINT32 GetTeamNum() { return m_mapPlayers.size(); }			//一边有多少人
	virtual UINT64* GetTeam() { return m_oPlayerSlots; }
	virtual UINT64* GetRedTeam() { return m_oPlayerSlots; }
	virtual UINT64* GetBlueTeam() { return m_oPlayerSlots + MAXCLIENTNUM / 2; }

	bool InsertIntoTeam(const GameProto::RoleData& refRoleData);
	bool KickPlayer(UINT64 qwPlayerId);

	UINT64 GetLeaderId() { return m_qwLeader; }
	ETeamState GetState() { return m_eState; }
	void SetState(ETeamState eState) { m_eState = eState; }

	GameProto::TeamRoleData* GetTeamRoleData(UINT64 qwPlayerId);
	std::map<UINT64, GameProto::TeamRoleData>& GetTeamRoles() { return m_mapPlayers; }

	std::map<UINT64, GameProto::TeamRoleData> m_mapPlayers;		//<playerid, data>

	UINT64 m_oPlayerSlots[MAXCLIENTNUM];						//playerid
	UINT64 m_qwLeader;

	ETeamState m_eState;
};

class CTeamManager
{
public:
	CTeamManager();
	~CTeamManager();

	CTeam* GetTeam(UINT64 qwTeamId);
	CTeam& CreateTeam(UINT64 qwTeamId);
	bool ReleaseTeam(UINT64 qwTeamId);
private:
	std::map<UINT64, CTeam> m_mapTeams;						//<teamid, team>

};

#endif // !__Team_H__
