#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatServerSession.h"
#include "GMSession.h"

class ChatServerManager : public TSingleton<ChatServerManager>
{
public:
	ChatServerManager();
	virtual ~ChatServerManager();

	bool Init(UINT32 dwPort, UINT32 dwGMPort);

	void Close();

	ChatServerSessionManager& GetChatServerSessionManager() { return m_oChatSessionManager; }
	IFxListenSocket* GetChatServerListenSocket() { return m_pChatServerListenSocket; }
	GMSessionManager& GetGMSessionManager() { return m_oGMSessionManager; }

private:
	ChatServerSessionManager m_oChatSessionManager;
	GMSessionManager m_oGMSessionManager;

	IFxListenSocket* m_pChatServerListenSocket;
	IFxListenSocket* m_pGMListenSocket;
};

#endif // !__GameServer_H__
