#ifndef __ChatPlayer_H__
#define __ChatPlayer_H__

#include <string>
#include "PlayerSession.h"

class Player
{
public:
	Player();
	virtual ~Player();

	bool Init(CPlayerSession* pSession, GameProto::PlayerRequestLogin& refLogin);
	CPlayerSession* GetSession() { return m_pSession; }

private:
	CPlayerSession* m_pSession;

	UINT64 m_qwPyayerId;
	std::string m_szNickName;
	std::string m_szAvatar;
	UINT32 m_dwSex;
	UINT32 m_dwBalance;
	std::string m_szToken;
};

#endif // !__ChatPlayer_H__

