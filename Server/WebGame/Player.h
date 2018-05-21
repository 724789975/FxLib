#ifndef __Player_H__
#define __Player_H__

#include "msg_proto/web_game.pb.h"

class CPlayerSession;
class CPlayerBase
{
public:
	CPlayerBase();
	virtual ~CPlayerBase();

	void SetPlayerSession(CPlayerSession* pPlayerSession) { m_pPlayerSession = pPlayerSession; }
	CPlayerSession* GetPlayerSession() { return m_pPlayerSession; }

	virtual void FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData) = 0;
	
protected:
	CPlayerSession* m_pPlayerSession;
};

class CCommonPlayer : public CPlayerBase
{
public:
	CCommonPlayer();

	~CCommonPlayer();

	virtual void FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData);

private:
	GameProto::GameCommonRoleData m_oGameRoleData;
};



#endif // __Player_H__
