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
	PlayrState_Game,		//游戏
};

class Player
{
public:
	Player();
	virtual ~Player();

	CPlayerSession* GetSession() { return m_pSession; }

	bool OnPlayerRequestLogin(CPlayerSession& refSession, GameProto::PlayerRequestLogin& refLogin);
	bool OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginMakeTeam& refMsg);
	bool OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, GameProto::PlayerRequestLoginInviteTeam& refMsg);
	bool OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, GameProto::PlayerRequestLoginChangeSlot& refMsg);

private:
	CPlayerSession* m_pSession;

	EPlayrState m_eState;

	UINT64 m_qwPyayerId;
	std::string m_szNickName;
	std::string m_szAvatar;
	UINT32 m_dwSex;
	UINT32 m_dwBalance;
	std::string m_szToken;
};

#endif // !__ChatPlayer_H__

