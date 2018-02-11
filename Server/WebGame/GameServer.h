#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>

#include "singleton.h"
#include "ServerSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	//ChatSession& GetChatSession() { return m_oChatSession; }
	CServerSession& GetServerSession() { return m_oServerSession; }

private:
	//ChatSession m_oChatSession;
	CWebSocketServerSession m_oServerSession;

	//std::vector<ChatSession> m_vecChatSession;
};


#endif // !__GameServer_H__
