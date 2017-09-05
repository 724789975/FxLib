#include "ChatServerSession.h"
#include "ChatServer.h"

const static unsigned int g_dwChatServerSessionBuffLen = 64 * 1024;
static char g_pChatServerSessionBuf[g_dwChatServerSessionBuffLen];

ChatServerSession::ChatServerSession()
	: m_dwHashIndex(0xFFFFFFFF)
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::OnConnect(void)
{
	//LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
	stCHAT_TO_CHAT_HASH_INDEX oCHAT_TO_CHAT_HASH_INDEX;
	oCHAT_TO_CHAT_HASH_INDEX.dwHashIndex = ChatServer::Instance()->GetChatServerSessionManager().GetHashIndex();
	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_TO_CHAT_HASH_INDEX);
	oCHAT_TO_CHAT_HASH_INDEX.Write(oStream);
	Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::CHAT_TO_CHAT_HASH_INDEX:	OnChatToChatHashIndex(pData, dwLen);	break;
		default:	Assert(0);	break;
	}
}

void ChatServerSession::Release(void)
{
	OnDestroy();
	//Init(NULL);
	//ChatServerSessionManager::Instance()->Release(this);
}

void ChatServerSession::OnChatToChatHashIndex(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_TO_CHAT_HASH_INDEX oCHAT_TO_CHAT_HASH_INDEX;
	oCHAT_TO_CHAT_HASH_INDEX.Read(oStream);
	m_dwHashIndex = oCHAT_TO_CHAT_HASH_INDEX.dwHashIndex;
	ChatServer::Instance()->GetChatServerSessionManager().SetHashIndex(m_dwHashIndex, this);
}

FxSession* ChatServerSessionManager::CreateSession()
{
	return GetChatServerSession();
}

ChatServerSession* ChatServerSessionManager::GetChatServerSession()
{
	m_oLock.Lock();
	ChatServerSession* pSession = NULL;
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

void ChatServerSessionManager::SetHashIndex(UINT32 dwIndex)
{
	m_dwHashIndex = dwIndex;
	for (unsigned int i = 0; i < ChatConstant::g_dwHashGen; ++i)
	{
		if (i % ChatConstant::g_dwChatServerNum == dwIndex)
		{
			m_setHashIndex.insert(i);
		}
	}
}

void ChatServerSessionManager::SetHashIndex(UINT32 dwIndex, ChatServerSession* pChatServerSession)
{
	for (unsigned int i = 0; i < ChatConstant::g_dwHashGen; ++i)
	{ 
		if (i % ChatConstant::g_dwChatServerNum == dwIndex)
		{
			m_mapSessionIpPort[i] = pChatServerSession;
		}
	}
}