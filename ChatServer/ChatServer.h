#ifndef __ChatServer_H__
#define __ChatServer_H__

#include <set>
#include "singleton.h"
#include "GameSession.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"
#include "ChatSession.h"
#include "ChatPlayerManager.h"

class ChatServer : public TSingleton<ChatServer>
{
public:
	ChatServer();
	virtual ~ChatServer();

	bool Init(std::string szChatSessionIp, UINT32 dwChatSessionPort, UINT32 dwChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }
	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatServerSessionManager; }
	ChatSessionManager& GetChatSessionManager() { return m_oChatSessionManager; }
	ChatPlayerManager& GetChatPlayerManager() { return m_oChatPlayerManager; }

	void Close();

	UINT32 GetChatSessionPort() { return m_dwChatSessionPort; }
	UINT32 GetChatServerSessionPort() { return m_dwChatServerSessionPort; }
	std::string GetChatSessionIp() { return m_szChatSessionIp; }

	void SetHashIndex(UINT32 dwIndex);
	UINT32 GetHashIndex() { return m_dwHashIndex; }

	bool CheckHashIndex(unsigned int dwIndex);
private:
	//GameSession m_oChatSession;
	ChatManagerSession m_oChatManagerSession;

	std::string m_szChatSessionIp;
	UINT32 m_dwChatSessionPort;
	UINT32 m_dwChatServerSessionPort;

	IFxListenSocket* m_pChatSessionListener;
	IFxListenSocket* m_pChatServerSessionListener;

	ChatServerSessionManager m_oChatServerSessionManager;
	ChatSessionManager m_oChatSessionManager;

	ChatPlayerManager m_oChatPlayerManager;

	std::set<unsigned int> m_setHashIndex;
	UINT32 m_dwHashIndex;
};

#endif // !__ChatServer_H__
