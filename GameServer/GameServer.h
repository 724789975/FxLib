#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	ChatSession& GetChatSession() { return m_oChatSession; }

private:
	ChatSession m_oChatSession;
};

#endif // !__GameServer_H__
