#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "GameSession.h"
#include "ChatManagerSession.h"

class ChatServer : public TSingleton<ChatServer>
{
public:
	ChatServer();
	virtual ~ChatServer();

	bool Init(UINT32 dwChatSessionPort, UINT32 dwChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }

private:
	//GameSession m_oChatSession;
	ChatManagerSession m_oChatManagerSession;

	UINT32 m_dwChatSessionPort;
	UINT32 m_dwChatServerSessionPort;
};

#endif // !__GameServer_H__
