#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatSession.h"

class ChatServerManager : public TSingleton<ChatServerManager>
{
public:
	ChatServerManager();
	virtual ~ChatServerManager();

	ChatServerSession& GetChatSession() { return m_oChatSession; }

private:
	ChatServerSession m_oChatSession;
};

#endif // !__GameServer_H__
