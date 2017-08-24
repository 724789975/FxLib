#include "ChatServer.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"


ChatServer::ChatServer()
	: m_dwChatSessionPort(0)
	, m_dwChatServerSessionPort(0)
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
	if (pNet->Listen(ChatServerSessionManager::Instance(), SLT_CommonTcp, 0, m_dwChatServerSessionPort) == false)
	{
		return false;
	}
	// todo
	//if (pNet->Listen(CSessionFactory::Instance(), SLT_CommonTcp, 0, m_dwChatSessionPort) == false)
	//{
	//	return false;
	//}

	return true;
}
