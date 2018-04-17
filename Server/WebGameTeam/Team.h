#ifndef __Team_H__
#define __Team_H__
#include "msg_proto/web_data.pb.h"
#include "gamedefine.h"
#include <map>

//后面可以加其他模式
class CTeam
{
public:
	CTeam();
	~CTeam();

	UINT32 GetTeamNum() { return MAXCLIENTNUM / 2; }			//一边有多少人
	virtual UINT64* GetRedTeam() { return m_oRoleSlots; }
	virtual UINT64* GetBlueTeam() { return m_oRoleSlots + MAXCLIENTNUM / 2; }

	std::map<UINT64, GameProto::TeamRoleData> m_mapRoles;		//<playerid, data>

	UINT64 m_oRoleSlots[MAXCLIENTNUM];							//playerid
};

#endif // !__Team_H__
