#ifndef __Player_H__
#define __Player_H__

#include "msg_proto/web_game.pb.h"
#include "TetrisBase.h"
#include "fxmeta.h"

class CPlayerSession;
class CPlayerBase
{
public:
	CPlayerBase();
	virtual ~CPlayerBase();

	virtual void Update(float fDelta) = 0;
	virtual void Init() = 0;

	void SetPlayerSession(CPlayerSession* pPlayerSession) { m_pPlayerSession = pPlayerSession; }
	CPlayerSession* GetPlayerSession() { return m_pPlayerSession; }

	void SetPlayerId(UINT64 qwPlayerId) { m_qwPlayerId = qwPlayerId; GetPlayerData(); }
	UINT64 GetPlayerId() { return m_qwPlayerId; }

	virtual TetrisBase& GetTetrisData() = 0;

	virtual void FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData) = 0;
	virtual void FillPlayerData(GameProto::RoleData& refRoleData) = 0;
	
protected:
	virtual void GetPlayerData() = 0;
	CPlayerSession* m_pPlayerSession;
	UINT64 m_qwPlayerId;
};

class CCommonPlayer : public CPlayerBase
{
public:
	CCommonPlayer();
	~CCommonPlayer();

	virtual void Update(float fDelta);
	virtual void Init();
	virtual TetrisBase& GetTetrisData() { return m_oTetrisData; }

	virtual void FillPlayerData(GameProto::GameNotifyPlayerGameRoleData& refRoleData);
	virtual void FillPlayerData(GameProto::RoleData& refRoleData);

protected:
	virtual void GetPlayerData();

private:
	GameProto::GameCommonRoleData m_oGameRoleData;
	CommonTetris m_oTetrisData;
};



#endif // __Player_H__
