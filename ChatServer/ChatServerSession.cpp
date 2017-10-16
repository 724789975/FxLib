#include "ChatServerSession.h"
#include "ChatServer.h"
#include "utility.h"
#include "ChatGroup.h"

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
	LogExe(LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
	stCHAT_TO_CHAT_HASH_INDEX oCHAT_TO_CHAT_HASH_INDEX;
	oCHAT_TO_CHAT_HASH_INDEX.dwHashIndex = ChatServer::Instance()->GetHashIndex();
	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_HASH_INDEX);
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
		case Protocol::CHAT_NOTIFY_CHAT_HASH_INDEX:			OnChatToChatHashIndex(pData, dwLen);	break;
		case Protocol::CHAT_NOTIFY_CHAT_PRIVATE_CHAT:		OnChatToChatPrivateChat(pData, dwLen);	break;
		case Protocol::CHAT_NOTIFY_CHAT_GROUP_CREATE:		OnChatToChatGroupCreate(pData, dwLen);	break;
		case Protocol::CHAT_NOTIFY_CHAT_GROUP_CHAT:			OnChatToChatGroupChat(pData, dwLen);	break;
		case Protocol::CHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT:	OnChatToChatGroupMemberChat(pData, dwLen);	break;
		default:	Assert(0);	break;
	}
}

void ChatServerSession::Release(void)
{
	OnDestroy();
	//Init(NULL);
	//ChatServerSessionManager::Instance()->Release(this);
}

void ChatServerSession::OnGroupCreate(unsigned int dwGroupId)
{
	stCHAT_NOTIFY_CHAT_GROUP_CREATE oCHAT_NOTIFY_CHAT_GROUP_CREATE;
	oCHAT_NOTIFY_CHAT_GROUP_CREATE.dwGroupId = dwGroupId;

	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_GROUP_CREATE);
	oCHAT_NOTIFY_CHAT_GROUP_CREATE.Write(oStream);
	Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSession::OnGroupMemberChat(stCHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT& refChat)
{
	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT);
	refChat.Write(oStream);
	Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSession::OnChatToChatHashIndex(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_TO_CHAT_HASH_INDEX oCHAT_TO_CHAT_HASH_INDEX;
	oCHAT_TO_CHAT_HASH_INDEX.Read(oStream);
	m_dwHashIndex = oCHAT_TO_CHAT_HASH_INDEX.dwHashIndex;
	ChatServer::Instance()->GetChatServerSessionManager().SetHashIndex(m_dwHashIndex, this);
}

void ChatServerSession::OnChatToChatPrivateChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_SEND_CHAT_PRIVATE_CHAT oCHAT_SEND_CHAT_PRIVATE_CHAT;
	oCHAT_SEND_CHAT_PRIVATE_CHAT.Read(oStream);

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oCHAT_SEND_CHAT_PRIVATE_CHAT.szRecverId, IDLENTH)))
	{
		//属于本服务器
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(oCHAT_SEND_CHAT_PRIVATE_CHAT.szRecverId);
		if (pPlayer)
		{
			pPlayer->OnPrivateChat(oCHAT_SEND_CHAT_PRIVATE_CHAT.szSenderId, oCHAT_SEND_CHAT_PRIVATE_CHAT.eChatType, oCHAT_SEND_CHAT_PRIVATE_CHAT.szContent, oCHAT_SEND_CHAT_PRIVATE_CHAT.dwTimeStamp);
		}
		DBChatQuery * pQuery = new DBChatQuery(oCHAT_SEND_CHAT_PRIVATE_CHAT, pPlayer != NULL);
		FxDBGetModule()->AddQuery(pQuery);
		return;
	}
}

void ChatServerSession::OnChatToChatGroupCreate(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_NOTIFY_CHAT_GROUP_CREATE oCHAT_NOTIFY_CHAT_GROUP_CREATE;
	oCHAT_NOTIFY_CHAT_GROUP_CREATE.Read(oStream);

	DBLoadGroupQuery* pQuery = new DBLoadGroupQuery(oCHAT_NOTIFY_CHAT_GROUP_CREATE.dwGroupId);
	FxDBGetModule()->AddQuery(pQuery);
}

void ChatServerSession::OnChatToChatGroupChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_NOTIFY_CHAT_GROUP_CHAT oCHAT_NOTIFY_CHAT_GROUP_CHAT;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.Read(oStream);

	DBGroupChatQuery* pQuery = new DBGroupChatQuery(oCHAT_NOTIFY_CHAT_GROUP_CHAT);
	FxDBGetModule()->AddQuery(pQuery);
}

void ChatServerSession::OnChatToChatGroupMemberChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT oChat;
	if (!oChat.Read(oStream))
	{
		LogExe(LogLv_Critical, "read error");
		return;
	}
	for (std::vector<std::string>::iterator it = oChat.vecPlayerIds.begin();
		it != oChat.vecPlayerIds.end(); ++it)
	{
		ChatPlayer* pChatPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(*it);
		pChatPlayer->OnGroupChat(oChat.oChat);
	}
}

FxSession* ChatServerSessionManager::CreateSession()
{
	m_oLock.Lock();
	ChatServerSession* pSession = NULL;
	for (int i = 0; i < ChatConstant::g_dwChatServerNum - 1; ++i)
	{
		if (m_oChatServerSessions[i].GetConnection() == NULL)
		{
			m_oChatServerSessions[i].Init((FxConnection*)0xFFFFFFFF);
			pSession = &m_oChatServerSessions[i];
			break;
		}
	}
	m_oLock.UnLock();
	return pSession;
}

ChatServerSession* ChatServerSessionManager::GetChatServerSession(unsigned int dwIndex)
{
	if (m_mapSessionIpPort.find(dwIndex) != m_mapSessionIpPort.end())
	{
		return m_mapSessionIpPort[dwIndex];
	}
	return NULL;
}

void ChatServerSessionManager::Release(FxSession* pSession)
{

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

void ChatServerSessionManager::CloseSessions()
{
	for (int i = 0; i < ChatConstant::g_dwChatServerNum - 1; ++i)
	{
		m_oChatServerSessions[i].Close();
	}
	bool bClosed = true;
	do
	{
		FxNetGetModule()->Run(0xffffffff);
		FxSleep(10);
		for (int i = 0; i < ChatConstant::g_dwChatServerNum - 1; ++i)
		{
			if (m_oChatServerSessions[i].GetConnection())
			{
				bClosed = false;
			}
		}
	} while (!bClosed);
}
