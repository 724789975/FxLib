#include "ChatServerManager.h"



ChatServerManager::ChatServerManager()
{
}


ChatServerManager::~ChatServerManager()
{
}

bool ChatServerManager::Init(UINT32 dwPort, UINT32 dwGMPort)
{
	m_pChatServerListenSocket = FxNetGetModule()->Listen(&m_oChatSessionManager, SLT_CommonTcp, 0, dwPort);
	if (m_pChatServerListenSocket == NULL)
	{
		return false;
	}
	if (m_oChatSessionManager.Init() == false)
	{
		return false;
	}

	m_pGMListenSocket = FxNetGetModule()->Listen(&m_oGMSessionManager, SLT_CommonTcp, 0, dwGMPort);
	if (m_pGMListenSocket == NULL)
	{
		return false;
	}
	if (m_oGMSessionManager.Init() == false)
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

	m_pGMListenSocket->StopListen();
	m_pGMListenSocket->Close();
}
