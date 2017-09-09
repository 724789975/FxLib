#include "ChatServerManager.h"



ChatServerManager::ChatServerManager()
{
}


ChatServerManager::~ChatServerManager()
{
}

bool ChatServerManager::Init(UINT32 dwPort)
{
	m_pChatServerListenSocket = FxNetGetModule()->Listen(&ChatServerManager::Instance()->GetChatSessionManager(), SLT_CommonTcp, 0, dwPort);
	if (m_pChatServerListenSocket == NULL)
	{
		return false;
	}
	if (m_oChatSessionManager.Init() == false)
	{
		return false;
	}
	return true;
}

void ChatServerManager::Close()
{
	m_pChatServerListenSocket->StopListen();
	m_pChatServerListenSocket->Close();

	m_oChatSessionManager.CloseSessions();
}
