#ifndef __Team_H__
#define __Team_H__
#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"
#include "gamedefine.h"
#include <map>
#include "fxmeta.h"

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

	unsigned int GetTeamNum() { return m_mapPlayers.size(); }			//一边有多少人
	virtual unsigned long long* GetTeam() { return m_pPlayerSlots; }
	virtual unsigned long long* GetRedTeam() { return m_pPlayerSlots; }
	virtual unsigned long long* GetBlueTeam() { return m_pPlayerSlots + MAXCLIENTNUM / 2; }

	bool InsertIntoTeam(const GameProto::RoleData& refRoleData);
	bool KickPlayer(unsigned long long qwPlayerId);

	void NotifyPlayer();

	unsigned long long GetLeaderId() { return m_qwLeader; }
	ETeamState GetState() { return m_eState; }
	
	void SetTeamId(unsigned long long qwTeamId) { m_qwTeamId = qwTeamId; }
	unsigned int GetTeamId(unsigned int dwTeamId) { return dwTeamId; }

	void SetState(ETeamState eState) { m_eState = eState; }

	GameProto::EErrorCode ChangeSlot(unsigned long long qwPlayerId, unsigned int dwSlotId);

	GameProto::TeamRoleData* GetTeamRoleData(unsigned long long qwPlayerId);
	std::map<unsigned long long, GameProto::TeamRoleData>& GetTeamRoles() { return m_mapPlayers; }

	std::map<unsigned long long, GameProto::TeamRoleData> m_mapPlayers;		//<playerid, data>
	unsigned long long m_pPlayerSlots[MAXCLIENTNUM];						//playerid
	unsigned long long m_qwLeader;

	ETeamState m_eState;
	unsigned long long m_qwTeamId;
};

class CTeamManager
{
public:
	CTeamManager();
	~CTeamManager();

	CTeam* GetTeam(unsigned long long qwTeamId);
	CTeam& CreateTeam(unsigned long long qwTeamId);
	bool ReleaseTeam(unsigned long long qwTeamId);
private:
	std::map<unsigned long long, CTeam> m_mapTeams;						//<teamid, team>

};

#endif // !__Team_H__
