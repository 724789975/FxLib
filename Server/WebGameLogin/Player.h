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
	PlayrState_GamePrepare,		//׼����Ϸ
	PlayrState_GameCountingSeconds,		//��Ϸ���� ��ԥ��ʱ��
	PlayrState_GamIng,		//��Ϸ��
};

class Player
{
public:
	Player();
	virtual ~Player();

	CPlayerSession* GetSession() { return m_pSession; }

	unsigned long long GetPlayerId() { return m_qwPyayerId; }
	unsigned long long GetTeamId() { return m_qwTeamId; }
	unsigned int GetTeamServerId() { return m_dwTeamServerId; }
	std::string GetNickName() { return m_szNickName; }
	std::string GetAvatar() { return m_szAvatar; }
	unsigned int GetSex() { return m_dwSex; }

	void SetState(EPlayrState eState) { m_eState = eState; }
	void SetTeamInfo(unsigned long long qwTeamId, unsigned int dwTeamServerId) { m_qwTeamId = qwTeamId; m_dwTeamServerId = dwTeamServerId; }

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

	unsigned long long m_qwPyayerId;
	std::string m_szNickName;
	std::string m_szAvatar;
	unsigned int m_dwSex;
	unsigned int m_dwBalance;
	std::string m_szToken;

	unsigned long long m_qwTeamId;
	unsigned int m_dwTeamServerId;
};

#endif // !__ChatPlayer_H__

