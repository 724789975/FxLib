#include "ChatPlayer.h"
#include "ChatServer.h"
#include "utility.h"
#include "fxdb.h"
#include <string>
#include <sstream>

const static unsigned int g_dwChatPlayerBuffLen = 64 * 1024;
static char g_pChatPlayerBuff[g_dwChatPlayerBuffLen];

ChatPlayer::ChatPlayer()
	: m_pSession(NULL)
{
	memset(m_szPyayerId, 0, IDLENTH);
}


ChatPlayer::~ChatPlayer()
{
}

bool ChatPlayer::Init(ChatSession* pSession, std::string szID)
{
	m_pSession = pSession;
	memcpy(m_szPyayerId, szID.c_str(), szID.size());

	return true;
}

void ChatPlayer::OnMsg(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oNetStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::PLAYER_REQUEST_PRIVATE_CHAT:	OnPrivateChat(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocot : %d", (unsigned int)eProrocol); m_pSession->Close(); }	break;
	}
}

class DBChatQuery : public IQuery
{
public:
	DBChatQuery() {}
	virtual ~DBChatQuery() {}

	virtual INT32 GetDBId(void) { return 0; }

	void Init(stCHAT_SENF_CHAT_PRIVATE_CHAT& oCHAT_SENF_CHAT_PRIVATE_CHAT)
	{
		char szTemp[2048 * 3 + 1] = { 0 };
		char szContentEacape[2048 * 3 + 1];
		mysql_escape_string(szContentEacape, oCHAT_SENF_CHAT_PRIVATE_CHAT.szContent.c_str(), oCHAT_SENF_CHAT_PRIVATE_CHAT.szContent.size());
		sprintf(szTemp, "INSERT INTO private_chat (`sender_id`, `recver_id`, `chat_type`, `content`, `send_time`) "
			"VALUES('%s', '%s', %u, '%s', %u)", oCHAT_SENF_CHAT_PRIVATE_CHAT.szSenderId,
			oCHAT_SENF_CHAT_PRIVATE_CHAT.szRecverId, (int)oCHAT_SENF_CHAT_PRIVATE_CHAT.eChatType,
			szContentEacape);
		m_strQuery = szContentEacape;
	}
	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		IDataReader* pReader = NULL;
		if (poDBConnection->Query(m_strQuery.c_str(), &pReader) == FXDB_HAS_RESULT)
		{
			pReader->Release();
		}
	}

	virtual void OnResult(void)
	{}

	virtual void Release(void)
	{
		delete this;
	}

	std::string m_strQuery;

private:

};

void ChatPlayer::OnPrivateChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_PRIVATE_CHAT oPLAYER_REQUEST_PRIVATE_CHAT;
	oPLAYER_REQUEST_PRIVATE_CHAT.Read(oNetStream);

	stCHAT_SENF_CHAT_PRIVATE_CHAT oCHAT_SENF_CHAT_PRIVATE_CHAT;
	memcpy(oCHAT_SENF_CHAT_PRIVATE_CHAT.szSenderId, m_szPyayerId, IDLENTH);
	memcpy(oCHAT_SENF_CHAT_PRIVATE_CHAT.szRecverId, oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH);
	oCHAT_SENF_CHAT_PRIVATE_CHAT.eChatType = oPLAYER_REQUEST_PRIVATE_CHAT.eChatType;
	oCHAT_SENF_CHAT_PRIVATE_CHAT.szContent = oPLAYER_REQUEST_PRIVATE_CHAT.szContent;

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH)))
	{
		//属于本服务器
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId);
		if (pPlayer)
		{
			CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
			oStream.WriteInt(Protocol::CHAT_SENF_CHAT_PRIVATE_CHAT);
			oCHAT_SENF_CHAT_PRIVATE_CHAT.Write(oStream);
			pPlayer->m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
			return;
		}
		DBChatQuery * pQuery = new DBChatQuery;
		pQuery->m_strQuery = pBuf;
		FxDBGetModule()->AddQuery(pQuery);
	}

	ChatServerSession* pChatServerSession = ChatServer::Instance()->GetChatServerSessionManager().GetChatServerSession(HashToIndex(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH));
	if (!pChatServerSession)
	{
		LogExe(LogLv_Critical, "can't find chat server session recver id : %s", oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId);
		return;
	}
	// todo

}
