#ifndef __ChatPlayer_H__
#define __ChatPlayer_H__

#include "ChatSession.h"
#include "chatdefine.h"

class ChatPlayer
{
public:
	ChatPlayer();
	virtual ~ChatPlayer();

	bool Init(ChatSession* pSession, std::string szID);
	ChatSession* GetSession() { return m_pSession; }

	void OnMsg(const char* pBuf, UINT32 dwLen);

	void OnPrivateChat(const char* pBuf, UINT32 dwLen);

private:
	ChatSession* m_pSession;

	char m_szPyayerId[IDLENTH];
};

#endif // !__ChatPlayer_H__

