#ifndef __GameServer_H__
#define __GameServer_H__

#include "singleton.h"
#include "ChatServerSession.h"

class ChatServerManager : public TSingleton<ChatServerManager>
{
public:
	ChatServerManager();
	virtual ~ChatServerManager();

	bool Init(UINT32 dwPort);

	void Close();

	ChatServerSessionManager& GetChatSessionManager() { return m_oChatSessionManager; }

	IFxListenSocket* GetChatServerListenSocket() { return m_pChatServerListenSocket; }

private:
	ChatServerSessionManager m_oChatSessionManager;
	IFxListenSocket* m_pChatServerListenSocket;
};

#endif // !__GameServer_H__
