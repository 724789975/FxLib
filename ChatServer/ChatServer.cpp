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

bool ChatServer::Init(std::string szChatSessionIp, UINT32 dwChatSessionPort, UINT32 dwChatServerSessionPort)
{
	m_oChatSessionManager.Init();

	m_szChatSessionIp = szChatSessionIp;
	m_dwChatServerSessionPort = dwChatServerSessionPort;
	m_dwChatSessionPort = dwChatSessionPort;
	IFxNet* pNet = FxNetGetModule();
	if (pNet == NULL)
	{
		return false;
	}
	struct hostent* pHost = gethostbyname(m_szChatSessionIp.c_str());
	unsigned int dwIp = 0;
	if (pHost)
	{
		for (int i = 0; (pHost->h_addr_list)[i] != NULL; i++)
		{
			dwIp = *(u_long*)pHost->h_addr_list[i];
		}
	}
	m_pChatSessionListener = pNet->Listen(&m_oChatSessionManager, SLT_CommonTcp, dwIp, m_dwChatSessionPort);
	if (m_pChatSessionListener  == NULL)
	{
		return false;
	}
	m_pChatServerSessionListener = pNet->Listen(&m_oChatServerSessionManager, SLT_CommonTcp, 0, m_dwChatServerSessionPort);
	if (m_pChatSessionListener == NULL)
	{
		return false;
	}

	return true;
}

void ChatServer::Close()
{
	m_pChatSessionListener->StopListen();
	m_pChatSessionListener->Close();

	m_pChatServerSessionListener->StopListen();
	m_pChatServerSessionListener->Close();

	m_oChatManagerSession.Close();
}

