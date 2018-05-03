#ifndef __ChatPlayer_H__
#define __ChatPlayer_H__

#include <string>
#include "PlayerSession.h"

enum EPlayrState
{
	PlayrState_None,
	PlayrState_Idle,		//大厅
	PlayrState_Login,		//登录 这个暂时不用
	PlayrState_MakeTeam,	//请求组队
	PlayrState_TeamCompleted,	//组队完成
	PlayrState_GamePrepare,		//准备游戏
	PlayrState_GameCountingSeconds,		//游戏读秒 犹豫的时间
	PlayrState_GamIng,		//游戏中
};

class Player
{
public:
	Player();
	virtual ~Player();

	CPlayerSession* GetSession() { return m_pSession; }

	UINT64 GetPlayerId() { return m_qwPyayerId; }
	UINT64 GetTeamId() { return m_qwTeamId; }
	std::string GetNickName() { return m_szNickName; }
	std::string GetAvatar() { return m_szAvatar; }
	UINT32 GetSex() { return m_dwSex; }

	void SetState(EPlayrState eState) { m_eState = eState; }
	void SetTeamInfo(UINT64 qwTeamId, UINT32 dwTeamServerId) { m_qwTeamId = qwTeamId; m_dwTeamServerId = dwTeamServerId; }

	bool OnPlayerRequestLogin(CPlayerSession& refSession, GameProto::PlayerRequestLogin& refLogin);
	void OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginMakeTeam& refMsg);
	bool OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginInviteTeam& refMsg);
	bool OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, GameProto::PlayerRequestLoginChangeSlot& refMsg);
	bool OnPlayerRequestLoginGameStart(CPlayerSession& refSession, GameProto::PlayerRequestLoginGameStart& refMsg);

	void OnTeamKick();

	void OnClose();

private:
	CPlayerSession* m_pSession;

	EPlayrState m_eState;

	UINT64 m_qwPyayerId;
	std::string m_szNickName;
	std::string m_szAvatar;
	UINT32 m_dwSex;
	UINT32 m_dwBalance;
	std::string m_szToken;

	UINT64 m_qwTeamId;
	UINT32 m_dwTeamServerId;
};

#endif // !__ChatPlayer_H__

