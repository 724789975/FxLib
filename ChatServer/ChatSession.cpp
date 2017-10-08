#include "ChatSession.h"
#include "ChatServer.h"
#include "utility.h"
#include "chatdefine.h"

const static unsigned int g_dwChatSessionBuffLen = 64 * 1024;
static char g_pChatSessionBuff[g_dwChatSessionBuffLen];

ChatSession::ChatSession()
{
	Reset();
}


ChatSession::~ChatSession()
{
}

void ChatSession::OnConnect(void)
{

}

void ChatSession::OnClose(void)
{
	ChatServer::Instance()->GetChatPlayerManager().OnSessionClose(m_szId);
}

void ChatSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, error no : %d\n", GetRemoteIPStr(), GetRemotePort(), dwErrorNo);
}

void ChatSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	ChatPlayer* pChatPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(m_szId);
	if (!pChatPlayer)
	{
		OnMsg(pBuf, dwLen);
	}
	else
	{
		pChatPlayer->OnMsg(pBuf, dwLen);
	}
}

void ChatSession::Reset()
{
	memset(m_szId, 0, IDLENTH);
}

void ChatSession::OnMsg(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oNetStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::PLAYER_REQUEST_CHAT_LOGIN:	OnLogin(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocol : %d", (unsigned int)eProrocol); Close(); }	break;
	}
}

class DBUnreadChatQuery : public IQuery
{
public:
	DBUnreadChatQuery(std::string szPlayerId)
	{
		m_pReader = NULL;
		m_szPlayerId = szPlayerId;
		static char szTemp[512] = { 0 };
		memset(szTemp, 0, 2048);
		sprintf(szTemp, "SELECT * FROM private_chat WHERE `readed` = 0 AND `recver_id` = '%s';"
			"UPDATE private_chat SET `readed` = 1 WHERE recver_id = '%s';",
			m_szPlayerId.c_str(), m_szPlayerId.c_str());
		m_strQuery = szTemp;
	}
	virtual ~DBUnreadChatQuery() {}

	virtual INT32 GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		if (poDBConnection->Query(m_strQuery.c_str(), &m_pReader) != FXDB_HAS_RESULT)
		{
			m_pReader->Release();
		}
	}

	virtual void OnResult(void)
	{
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(m_szPlayerId);
		while (m_pReader->GetNextRecord())
		{
			//stCHAT_SEND_PLAYER_PRIVATE_CHAT oCHAT_SEND_PLAYER_PRIVATE_CHAT;
			//memcpy(oCHAT_SEND_PLAYER_PRIVATE_CHAT.szSenderId, m_pReader->GetFieldValue(0), IDLENTH);
			//memcpy(oCHAT_SEND_PLAYER_PRIVATE_CHAT.szRecverId, m_pReader->GetFieldValue(1), IDLENTH);
			//oCHAT_SEND_PLAYER_PRIVATE_CHAT.eChatType = (Protocol::EChatType)atoi(m_pReader->GetFieldValue(2));
			//oCHAT_SEND_PLAYER_PRIVATE_CHAT.szContent = m_pReader->GetFieldValue(3);
			//oCHAT_SEND_PLAYER_PRIVATE_CHAT.dwTimeStamp = atoi(m_pReader->GetFieldValue(4));
			
			pPlayer->OnPrivateChat(m_pReader->GetFieldValue(1), (Protocol::EChatType)atoi(m_pReader->GetFieldValue(3)), m_pReader->GetFieldValue(4), atoi(m_pReader->GetFieldValue(5)));
		}
	}

	virtual void Release(void)
	{
		m_pReader->Release();
		delete this;
	}
private:
	std::string m_szPlayerId;
	std::string m_strQuery;
	IDataReader* m_pReader;
};

void ChatSession::OnLogin(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_CHAT_LOGIN oPLAYER_REQUEST_CHAT_LOGIN;
	oPLAYER_REQUEST_CHAT_LOGIN.Read(oNetStream);

	stCHAT_ACK_PLAYER_LOGIN oCHAT_ACK_PLAYER_LOGIN;
	oCHAT_ACK_PLAYER_LOGIN.dwResult = 1;
	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oPLAYER_REQUEST_CHAT_LOGIN.szId, strlen(oPLAYER_REQUEST_CHAT_LOGIN.szId))))
	{
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().OnPlayerLogin(oPLAYER_REQUEST_CHAT_LOGIN.szId, oPLAYER_REQUEST_CHAT_LOGIN.szSign, this);
		if (pPlayer)
		{
			oCHAT_ACK_PLAYER_LOGIN.dwResult = 0;
			memcpy(m_szId, oPLAYER_REQUEST_CHAT_LOGIN.szId, IDLENTH);

			DBUnreadChatQuery* pQuery = new DBUnreadChatQuery(oPLAYER_REQUEST_CHAT_LOGIN.szId);
			FxDBGetModule()->AddQuery(pQuery);
		}
	}
	CNetStream oStream(ENetStreamType_Write, g_pChatSessionBuff, g_dwChatSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_ACK_PLAYER_LOGIN);
	
	oCHAT_ACK_PLAYER_LOGIN.Write(oStream);
	Send(g_pChatSessionBuff, g_dwChatSessionBuffLen - oStream.GetDataLength());
}
//--------------------------------------------------------------
void ChatBinarySession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
	Reset();

	ChatServer::Instance()->GetChatBinarySessionManager().Release(this);
}

//--------------------------------------------------------------
FxSession* ChatBinarySessionManager::CreateSession()
{
	m_oLock.Lock();
	ChatBinarySession* pSession = m_poolSessions.FetchObj();
	m_oLock.UnLock();
	return pSession;
}

bool ChatBinarySessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void ChatBinarySessionManager::Release(FxSession* pSession)
{
	Assert(0);
	//m_poolSessions.ReleaseObj()
}

void ChatBinarySessionManager::Release(ChatBinarySession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

//--------------------------------------------------------------

void ChatWebSocketSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
	Reset();

	ChatServer::Instance()->GetChatWebSocketSessionManager().Release(this);
}
//--------------------------------------------------------------

FxSession* ChatWebSocketSessionManager::CreateSession()
{
	return m_poolSessions.FetchObj();
}

bool ChatWebSocketSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void ChatWebSocketSessionManager::Release(FxSession* pSession)
{
	Assert(0);
}

void ChatWebSocketSessionManager::Release(ChatWebSocketSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}
