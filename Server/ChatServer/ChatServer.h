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

	bool Init(std::string szChatSessionIp, UINT16& wChatSessionPort, UINT16& wChatWebSocketSessionPort, UINT16& wChatServerSessionPort);

	//GameSession& GetGameSession() { return m_oChatSession; }
	ChatManagerSession& GetChatManagerSession() { return m_oChatManagerSession; }
	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatServerSessionManager; }
	ChatBinarySessionManager& GetChatBinarySessionManager() { return m_oChatBinarySessionManager; }
	ChatWebSocketSessionManager& GetChatWebSocketSessionManager() { return m_oChatWebSocketSessionManager; }
	ChatPlayerManager& GetChatPlayerManager() { return m_oChatPlayerManager; }
	ChatGroupManager& GetChatGroupManager() { return m_oChatGroupManager; }

	void Close();

	UINT32 GetChatSessionPort() { return m_wChatSessionPort; }
	UINT32 GetChatWebSocketSessionPort() { return m_wChatWebSocketSessionPort; }
	UINT32 GetChatServerSessionPort() { return m_wChatServerSessionPort; }
	std::string GetChatSessionIp() { return m_szChatSessionIp; }

	void SetHashIndex(UINT32 dwIndex);
	UINT32 GetHashIndex() { return m_dwHashIndex; }

	bool CheckHashIndex(unsigned int dwIndex);
private:
	//GameSession m_oChatSession;
	ChatManagerSession m_oChatManagerSession;

	std::string m_szChatSessionIp;
	UINT16 m_wChatSessionPort;
	UINT16 m_wChatWebSocketSessionPort;
	UINT16 m_wChatServerSessionPort;

	IFxListenSocket* m_pChatSessionListener;
	IFxListenSocket* m_pChatWebSocketSessionListener;
	IFxListenSocket* m_pChatServerSessionListener;

	ChatServerSessionManager m_oChatServerSessionManager;
	ChatBinarySessionManager m_oChatBinarySessionManager;
	ChatWebSocketSessionManager m_oChatWebSocketSessionManager;

	ChatPlayerManager m_oChatPlayerManager;
	
	ChatGroupManager m_oChatGroupManager;

	std::set<unsigned int> m_setHashIndex;
	UINT32 m_dwHashIndex;
};

#endif // !__ChatServer_H__
