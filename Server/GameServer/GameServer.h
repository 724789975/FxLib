#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>

#include "singleton.h"
#include "ChatSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	//ChatSession& GetChatSession() { return m_oChatSession; }

private:
	//ChatSession m_oChatSession;

	std::vector<ChatSession> m_vecChatSession;
};


#endif // !__GameServer_H__
