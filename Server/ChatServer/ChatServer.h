#ifndef __ChatServer_H__
#define __ChatServer_H__

#include <set>
#include "singleton.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"
#include "ChatSession.h"
#include "ChatPlayerManager.h"
#include "ChatGroup.h"

class ChatServer : public TSingleton<ChatServer>
{
public:
	ChatServer();
	virtual ~ChatServer();

	bool Init(std::string szChatSessionIp, unsigned short& wChatSessionPort, unsigned short& wChatWebSocketSessionPort, unsigned short& wChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }
	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatServerSessionManager; }
	ChatBinarySessionManager& GetChatBinarySessionManager() { return m_oChatBinarySessionManager; }
	ChatWebSocketSessionManager& GetChatWebSocketSessionManager() { return m_oChatWebSocketSessionManager; }
	ChatPlayerManager& GetChatPlayerManager() { return m_oChatPlayerManager; }
	ChatGroupManager& GetChatGroupManager() { return m_oChatGroupManager; }

	void Close();

	unsigned int GetChatSessionPort() { return m_wChatSessionPort; }
	unsigned int GetChatWebSocketSessionPort() { return m_wChatWebSocketSessionPort; }
	unsigned int GetChatServerSessionPort() { return m_wChatServerSessionPort; }
	std::string GetChatSessionIp() { return m_szChatSessionIp; }

	void SetHashIndex(unsigned int dwIndex);
	unsigned int GetHashIndex() { return m_dwHashIndex; }

	bool CheckHashIndex(unsigned int dwIndex);
private:
	//GameSession m_oChatSession;
	ChatManagerSession m_oChatManagerSession;

	std::string m_szChatSessionIp;
	unsigned short m_wChatSessionPort;
	unsigned short m_wChatWebSocketSessionPort;
	unsigned short m_wChatServerSessionPort;

	IFxListenSocket* m_pChatSessionListener;
	IFxListenSocket* m_pChatWebSocketSessionListener;
	IFxListenSocket* m_pChatServerSessionListener;

	ChatServerSessionManager m_oChatServerSessionManager;
	ChatBinarySessionManager m_oChatBinarySessionManager;
	ChatWebSocketSessionManager m_oChatWebSocketSessionManager;

	ChatPlayerManager m_oChatPlayerManager;
	
	ChatGroupManager m_oChatGroupManager;

	std::set<unsigned int> m_setHashIndex;
	unsigned int m_dwHashIndex;
};

#endif // !__ChatServer_H__
