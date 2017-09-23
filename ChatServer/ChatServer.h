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

	bool Init(std::string szChatSessionIp, UINT32 dwChatSessionPort, UINT32 dwChatWebSocketSessionPort, UINT32 dwChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }
	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatServerSessionManager; }
	ChatBinarySessionManager& GetChatBinarySessionManager() { return m_oChatBinarySessionManager; }
	ChatWebSocketSessionManager& GetChatWebSocketSessionManager() { return m_oChatWebSocketSessionManager; }
	ChatPlayerManager& GetChatPlayerManager() { return m_oChatPlayerManager; }

	void Close();

	UINT32 GetChatSessionPort() { return m_dwChatSessionPort; }
	UINT32 GetChatWebSocketSessionPort() { return m_dwChatWebSocketSessionPort; }
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
	UINT32 m_dwChatWebSocketSessionPort;
	UINT32 m_dwChatServerSessionPort;

	IFxListenSocket* m_pChatSessionListener;
	IFxListenSocket* m_pChatWebSocketSessionListener;
	IFxListenSocket* m_pChatServerSessionListener;

	ChatServerSessionManager m_oChatServerSessionManager;
	ChatBinarySessionManager m_oChatBinarySessionManager;
	ChatWebSocketSessionManager m_oChatWebSocketSessionManager;

	ChatPlayerManager m_oChatPlayerManager;

	std::set<unsigned int> m_setHashIndex;
	UINT32 m_dwHashIndex;
};

#endif // !__ChatServer_H__
