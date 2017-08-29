#include "ChatServerSession.h"
#include "netstream.h"

ChatServerSession::ChatServerSession()
	: m_dwChatPort(0)
	, m_dwChatServerPort(0)
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::OnConnect(void)
{

}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatServerSession::Release(void)
{
}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	Protocol::EChatProtocol eProrocol = (Protocol::EChatProtocol)(*((UINT32*)pBuf));
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::CHAT_SEND_CHAT_MANAGER_INFO:	OnChatServerInfo(pData, dwLen);	break;
		default:	Assert(0);	break;
	}
}

void ChatServerSession::OnChatServerInfo(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	oStream.ReadInt(m_dwChatPort);
	oStream.ReadInt(m_dwChatServerPort);
}

//----------------------------------------------------------------------
FxSession* ChatServerSessionManager::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	for (int i = 0; i < ChatConstant::g_dwChatServerNum; ++i)
	{
		if (m_oChatServerSessions[i].GetConnection() == NULL)
		{
			pSession = &m_oChatServerSessions[i];
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void ChatServerSessionManager::Release(FxSession* pSession)
{

}

void ChatServerSessionManager::OnChatServerInfo(ChatServerSession* pChatServerSession)
{
	for (unsigned int i = 0; i < ChatConstant::g_dwChatServerNum; ++i)
	{
		if (pChatServerSession == m_oChatServerSessions + i)
		{
			// 同一个连接
			for (unsigned int j = 0; j < ChatConstant::g_dwHashGen; ++j)
			{
				if (j % ChatConstant::g_dwHashGen == i)
				{
					m_mapSessionIpPort[j] = pChatServerSession;
				}
			}
		}
		else
		{
			if (m_oChatServerSessions[i].m_dwChatPort && m_oChatServerSessions[i].m_dwChatServerPort)
			{
				// todo 发送这个服的信息
				//pChatServerSession->Send()
			}
		}
	}
	
}
