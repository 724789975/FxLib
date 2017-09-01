#ifndef __GameServer_H__
#define __GameServer_H__

#include <set>
#include "singleton.h"
#include "GameSession.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"

class ChatServer : public TSingleton<ChatServer>
{
public:
	ChatServer();
	virtual ~ChatServer();

	bool Init(UINT32 dwChatSessionPort, UINT32 dwChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }
	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatServerSessionManager; }

private:
	//GameSession m_oChatSession;
	ChatManagerSession m_oChatManagerSession;

	UINT32 m_dwChatSessionPort;
	UINT32 m_dwChatServerSessionPort;

	IFxListenSocket* m_pChatSessionListener;
	IFxListenSocket* m_pChatServerSessionListener;

	ChatServerSessionManager m_oChatServerSessionManager;

	std::set<char> m_setHashIndex;
};

#endif // !__GameServer_H__
