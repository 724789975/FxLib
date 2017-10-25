#include "ChatServer.h"
#include "ChatManagerSession.h"
#include "ChatServerSession.h"

#ifndef WIN32
#include <netdb.h>
#include <sys/socket.h>
#endif


ChatServer::ChatServer()
	: m_dwChatSessionPort(0)
	, m_dwChatServerSessionPort(0)
	, m_pChatSessionListener(NULL)
	, m_pChatServerSessionListener(NULL)
	, m_dwHashIndex(0xFFFFFFFF)
{
}

ChatServer::~ChatServer()
{
}

bool ChatServer::Init(std::string szChatSessionIp, UINT32 dwChatSessionPort, UINT32 dwChatWebSocketSessionPort, UINT32 dwChatServerSessionPort)
{
	m_oChatBinarySessionManager.Init();
	m_oChatPlayerManager.Init();
	m_oChatServerSessionManager.Init();
	m_oChatWebSocketSessionManager.Init();

	m_szChatSessionIp = szChatSessionIp;
	m_dwChatServerSessionPort = dwChatServerSessionPort;
	m_dwChatSessionPort = dwChatSessionPort;
	m_dwChatWebSocketSessionPort = dwChatWebSocketSessionPort;
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
	m_pChatServerSessionListener = pNet->Listen(&m_oChatServerSessionManager, SLT_CommonTcp, 0, m_dwChatServerSessionPort);
	if (m_pChatServerSessionListener == NULL)
	{
		return false;
	}
	m_pChatSessionListener = pNet->Listen(&m_oChatBinarySessionManager, SLT_CommonTcp, dwIp, m_dwChatSessionPort);
	if (m_pChatSessionListener  == NULL)
	{
		return false;
	}
	m_pChatWebSocketSessionListener = pNet->Listen(&m_oChatWebSocketSessionManager, SLT_WebSocket, dwIp, dwChatWebSocketSessionPort);
	if (m_pChatWebSocketSessionListener == NULL)
	{
		return false;
	}

	return true;
}

void ChatServer::Close()
{
	m_pChatSessionListener->StopListen();
	m_pChatSessionListener->Close();

	m_pChatWebSocketSessionListener->StopListen();
	m_pChatWebSocketSessionListener->Close();

	m_pChatServerSessionListener->StopListen();
	m_pChatServerSessionListener->Close();

	m_oChatManagerSession.Close();
}

void ChatServer::SetHashIndex(UINT32 dwIndex)
{
	m_dwHashIndex = dwIndex;
	for (unsigned int i = 0; i < ChatConstant::g_dwHashGen; ++i)
	{
		if (i % CHAT_SERVER_NUM == dwIndex)
		{
			m_setHashIndex.insert(i);
		}
	}

	//设置了hashindex后 才加载群聊
	m_oChatGroupManager.Init();
}

bool ChatServer::CheckHashIndex(unsigned int dwIndex)
{
	if (m_setHashIndex.find(dwIndex) == m_setHashIndex.end())
	{
		return false;
	}
	return true;
}

