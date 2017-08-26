#include "ChatServer.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"


ChatServer::ChatServer()
	: m_dwChatSessionPort(0)
	, m_dwChatServerSessionPort(0)
	, m_pChatSessionListener(NULL)
	, m_pChatServerSessionListener(NULL)
{
}


ChatServer::~ChatServer()
{
}

bool ChatServer::Init(UINT32 dwChatSessionPort, UINT32 dwChatServerSessionPort)
{
	m_dwChatServerSessionPort = dwChatServerSessionPort;
	m_dwChatSessionPort = dwChatSessionPort;
	IFxNet* pNet = FxNetGetModule();
	if (pNet == NULL)
	{
		return false;
	}
	m_pChatSessionListener = pNet->Listen(ChatServerSessionManager::Instance(), SLT_CommonTcp, 0, m_dwChatSessionPort);
	if (m_pChatSessionListener  == NULL)
	{
		return false;
	}
	m_pChatServerSessionListener = pNet->Listen(ChatServerSessionManager::Instance(), SLT_CommonTcp, 0, m_dwChatServerSessionPort);
	if (m_pChatSessionListener == NULL)
	{
		return false;
	}

	return true;
}
