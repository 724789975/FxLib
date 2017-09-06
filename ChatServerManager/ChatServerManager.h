#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatServerSession.h"

class ChatServerManager : public TSingleton<ChatServerManager>
{
public:
	ChatServerManager();
	virtual ~ChatServerManager();

	bool Init();

	ChatServerSessionManager& GetChatSessionManager() { return m_oChatSessionManager; }

private:
	ChatServerSessionManager m_oChatSessionManager;
};

#endif // !__GameServer_H__
