#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "GameSession.h"

class ChatServer : public TSingleton<ChatServer>
{
public:
	ChatServer();
	virtual ~ChatServer();

	GameSession& GetChatSession() { return m_oChatSession; }

private:
	GameSession m_oChatSession;
};

#endif // !__GameServer_H__
