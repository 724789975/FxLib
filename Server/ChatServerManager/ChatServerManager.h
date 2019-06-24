#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatServerSession.h"
#include "GMSession.h"
#include "GameSession.h"

class ChatServerManager : public TSingleton<ChatServerManager>
{
public:
	ChatServerManager();
	virtual ~ChatServerManager();

	bool Init(unsigned short& dwPort, unsigned short& dwGamePort, unsigned short& dwGMPort);

	void Close();

	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatSessionManager; }
	IFxListenSocket* GetChatServerListenSocket() { return m_pChatServerListenSocket; }
	GMSessionManager& GetGMSessionManager() { return m_oGMSessionManager; }
	GameSessionManager& GetGameSessionManager() { return m_oGameSessionManager; }

private:
	ChatServerSessionManager m_oChatSessionManager;
	GMSessionManager m_oGMSessionManager;
	
	GameSessionManager m_oGameSessionManager;

	IFxListenSocket* m_pChatServerListenSocket;
	IFxListenSocket* m_pGameServerListenSocket;
	IFxListenSocket* m_pGMListenSocket;
};

#endif // !__GameServer_H__
