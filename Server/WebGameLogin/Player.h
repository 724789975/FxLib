#ifndef __ChatPlayer_H__
#define __ChatPlayer_H__

#include <string>
#include "PlayerSession.h"

enum EPlayrState
{
	PlayrState_None,
	PlayrState_Idle,		//����
	PlayrState_Login,		//��¼ �����ʱ����
	PlayrState_MakeTeam,	//�������
	PlayrState_TeamCompleted,	//������
	PlayrState_Game,		//��Ϸ
};

class Player
{
public:
	Player();
	virtual ~Player();

	CPlayerSession* GetSession() { return m_pSession; }

	void SetState(EPlayrState eState) { m_eState = eState; }
	void SetTeamId(UINT64 qwTeamId) { m_qwTeamId = qwTeamId; }

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

	UINT64 m_qwTeamId;
};

#endif // !__ChatPlayer_H__

