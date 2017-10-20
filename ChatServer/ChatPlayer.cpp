#include "ChatPlayer.h"
#include "ChatServer.h"
#include "utility.h"
#include "fxdb.h"
#include <string>
#include <sstream>
#include "ChatGroup.h"

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
		case Protocol::PLAYER_REQUEST_CREATE_CHAT_GROUP:	OnRequestCreateChatGroup(pData, dwLen);	break;
		case Protocol::PLAYER_REQUEST_CHAT_GROUP_CHAT:	OnRequestGroupChat(pData, dwLen);	break;
		case Protocol::PLAYER_REQUEST_INVITE_ENTER_GROUP_CHAT:	OnRequestInviteEnterGroupChat(pData, dwLen);	break;
		case Protocol::PLAYER_REQUEST_LEAVE_GROUP_CHAT:	OnRequestLeaveGroupChat(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocol : %d", (unsigned int)eProrocol); m_pSession->Close(); }	break;
	}
}

void ChatPlayer::OnPrivateChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_PRIVATE_CHAT oPLAYER_REQUEST_PRIVATE_CHAT;
	oPLAYER_REQUEST_PRIVATE_CHAT.Read(oNetStream);

	stCHAT_SEND_PLAYER_PRIVATE_CHAT oCHAT_SEND_PLAYER_PRIVATE_CHAT;
	memcpy(oCHAT_SEND_PLAYER_PRIVATE_CHAT.szSenderId, m_szPyayerId, IDLENTH);
	memcpy(oCHAT_SEND_PLAYER_PRIVATE_CHAT.szRecverId, oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH);
	oCHAT_SEND_PLAYER_PRIVATE_CHAT.eChatType = oPLAYER_REQUEST_PRIVATE_CHAT.eChatType;
	oCHAT_SEND_PLAYER_PRIVATE_CHAT.szContent = oPLAYER_REQUEST_PRIVATE_CHAT.szContent;
	oCHAT_SEND_PLAYER_PRIVATE_CHAT.dwTimeStamp = GetTimeHandler()->GetSecond();
	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH)))
	{
		//属于本服务器
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId);
		if (pPlayer)
		{
			if (pPlayer == this)
			{
				LogExe(LogLv_Critical, "send to self");
				return;
			}
			CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
			oStream.WriteInt(Protocol::CHAT_NOTIFY_PLAYER_PRIVATE_CHAT);
			oCHAT_SEND_PLAYER_PRIVATE_CHAT.Write(oStream);
			pPlayer->m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
		}
		DBChatQuery * pQuery = new DBChatQuery(oCHAT_SEND_PLAYER_PRIVATE_CHAT, pPlayer != NULL);
		FxDBGetModule()->AddQuery(pQuery);
		return;
	}

	stCHAT_SEND_CHAT_PRIVATE_CHAT& oCHAT_SEND_CHAT_PRIVATE_CHAT = oCHAT_SEND_PLAYER_PRIVATE_CHAT;

	ChatServerSession* pChatServerSession = ChatServer::Instance()->GetChatServerSessionManager().GetChatServerSession(HashToIndex(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH));
	if (!(pChatServerSession && pChatServerSession->GetConnection()))
	{
		DBChatQuery * pQuery = new DBChatQuery(oCHAT_SEND_CHAT_PRIVATE_CHAT, false);
		FxDBGetModule()->AddQuery(pQuery);
		LogExe(LogLv_Critical, "can't find chat server session recver id : %s", oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId);
		return;
	}

	CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_PRIVATE_CHAT);
	oCHAT_SEND_CHAT_PRIVATE_CHAT.Write(oStream);
	pChatServerSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}

void ChatPlayer::OnPrivateChat(const char* szSender, const Protocol::EChatType eChatType, const std::string szContent, unsigned int dwTimeStamp)
{
	stCHAT_SEND_CHAT_PRIVATE_CHAT oCHAT_SEND_CHAT_PRIVATE_CHAT;
	oCHAT_SEND_CHAT_PRIVATE_CHAT.dwTimeStamp = dwTimeStamp;
	memcpy(oCHAT_SEND_CHAT_PRIVATE_CHAT.szSenderId, szSender, IDLENTH);
	oCHAT_SEND_CHAT_PRIVATE_CHAT.eChatType = eChatType;
	oCHAT_SEND_CHAT_PRIVATE_CHAT.szContent = szContent;
	memcpy(oCHAT_SEND_CHAT_PRIVATE_CHAT.szRecverId, m_szPyayerId, IDLENTH);

	CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_PLAYER_PRIVATE_CHAT);
	oCHAT_SEND_CHAT_PRIVATE_CHAT.Write(oStream);
	m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}

class DBCreateGroupQuery : public IQuery
{
public:
	DBCreateGroupQuery(std::string szPlayerId)
	{
		m_pReader = NULL;
		m_szPlayerId = szPlayerId;
		static char szTemp[512] = { 0 };
		memset(szTemp, 0, 512);
		sprintf(szTemp, "CALL create_group('%s');", m_szPlayerId.c_str());
		m_strQuery = szTemp;
	}
	~DBCreateGroupQuery(){}
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
		UINT32 dwGroupId = 0;
		while (m_pReader->GetNextRecord())
		{
			dwGroupId = atoi(m_pReader->GetFieldValue(0));
			m_pReader->GetFieldValue(1);
		}
		ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(m_szPlayerId);
		if (pPlayer)
		{
			pPlayer->OnCreateChatGroup(dwGroupId);
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

void ChatPlayer::OnRequestCreateChatGroup(const char* pBuf, UINT32 dwLen)
{
	DBCreateGroupQuery* pQuery = new DBCreateGroupQuery(m_szPyayerId);
	FxDBGetModule()->AddQuery(pQuery);
}

void ChatPlayer::OnCreateChatGroup(UINT32 dwGroupId)
{
	if (dwGroupId == 0)
	{
		LogExe(LogLv_Error, "group id == 0");
		return;
	}
	stCHAT_ACK_PLAYER_CREATE_CHAT_GROUP oCHAT_ACK_PLAYER_CREATE_CHAT_GROUP;
	oCHAT_ACK_PLAYER_CREATE_CHAT_GROUP.dwGroupId = dwGroupId;

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(dwGroupId)))
	{
		DBLoadGroupQuery* pQuery = new DBLoadGroupQuery(dwGroupId);
		FxDBGetModule()->AddQuery(pQuery);
	}
	else
	{
		ChatServerSessionManager& refServerSessionManager = ChatServer::Instance()->GetChatServerSessionManager();
		ChatServerSession* pChatServerSession = refServerSessionManager.GetChatServerSession(HashToIndex(dwGroupId));
		if (pChatServerSession)
		{
			pChatServerSession->OnGroupCreate(dwGroupId);
		}
	}

	CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
	oStream.WriteInt(Protocol::CHAT_ACK_PLAYER_CREATE_CHAT_GROUP);
	oCHAT_ACK_PLAYER_CREATE_CHAT_GROUP.Write(oStream);
	m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}

void ChatPlayer::OnRequestGroupChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_CHAT_GROUP_CHAT oPLAYER_REQUEST_CHAT_GROUP_CHAT;
	oPLAYER_REQUEST_CHAT_GROUP_CHAT.Read(oNetStream);

	stCHAT_NOTIFY_CHAT_GROUP_CHAT oCHAT_NOTIFY_CHAT_GROUP_CHAT;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.dwGroupId = oPLAYER_REQUEST_CHAT_GROUP_CHAT.dwGroupId;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.szSenderId = m_szPyayerId;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.eChatType = oPLAYER_REQUEST_CHAT_GROUP_CHAT.eChatType;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.szContent = oPLAYER_REQUEST_CHAT_GROUP_CHAT.szContent;
	oCHAT_NOTIFY_CHAT_GROUP_CHAT.dwSendTime = GetTimeHandler()->GetSecond();

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oPLAYER_REQUEST_CHAT_GROUP_CHAT.dwGroupId)))
	{
		ChatGroup* pGroup = ChatServer::Instance()->GetChatGroupManager().GetChatGroup(oPLAYER_REQUEST_CHAT_GROUP_CHAT.dwGroupId);
		if (!pGroup)
		{
			LogExe(LogLv_Error, "can't find group id : %d", oPLAYER_REQUEST_CHAT_GROUP_CHAT.dwGroupId);
			return;
		}

		DBGroupChatQuery* pQuery = new DBGroupChatQuery(oCHAT_NOTIFY_CHAT_GROUP_CHAT);
		FxDBGetModule()->AddQuery(pQuery);
	}
	else
	{
		ChatServerSessionManager& refServerSessionManager = ChatServer::Instance()->GetChatServerSessionManager();
		ChatServerSession* pChatServerSession = refServerSessionManager.GetChatServerSession(HashToIndex(oPLAYER_REQUEST_CHAT_GROUP_CHAT.dwGroupId));
		if (pChatServerSession)
		{
			CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
			oStream.WriteInt(Protocol::CHAT_NOTIFY_CHAT_GROUP_CHAT);
			oCHAT_NOTIFY_CHAT_GROUP_CHAT.Write(oStream);
			pChatServerSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
		}
	}
}

void ChatPlayer::OnGroupChat(stCHAT_NOTIFY_PLAYER_GROUP_CHAT& refChat)
{
	CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
	oStream.WriteInt(Protocol::CHAT_NOTIFY_PLAYER_GROUP_CHAT);
	refChat.Write(oStream);
	m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}

void ChatPlayer::OnRequestInviteEnterGroupChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_INVITE_ENTER_GROUP_CHAT oInviteEnterGroupChat;
	oInviteEnterGroupChat.Read(oNetStream);

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oInviteEnterGroupChat.dwGroupId)))
	{
		ChatGroup* pGroup = ChatServer::Instance()->GetChatGroupManager().GetChatGroup(oInviteEnterGroupChat.dwGroupId);
		if (!pGroup)
		{
			LogExe(LogLv_Error, "can't find group id : %d", oInviteEnterGroupChat.dwGroupId);
			return;
		}
		pGroup->OnInviteMember(m_szPyayerId, oInviteEnterGroupChat.szPlayerId);
	}
	else
	{
		stCHAT_NOTIFY_CHAT_INVITE_ENTER_GROUP_CHAT oC2CInviteEnterGroupChat;
		oC2CInviteEnterGroupChat.dwGroupId = oInviteEnterGroupChat.dwGroupId;
		oC2CInviteEnterGroupChat.szPlayerId = oInviteEnterGroupChat.szPlayerId;
		oC2CInviteEnterGroupChat.szInviter = m_szPyayerId;
		ChatServerSessionManager& refServerSessionManager = ChatServer::Instance()->GetChatServerSessionManager();
		ChatServerSession* pChatServerSession = refServerSessionManager.GetChatServerSession(HashToIndex(oInviteEnterGroupChat.dwGroupId));
		if (pChatServerSession)
		{
			pChatServerSession->OnInviteGroupMember(oC2CInviteEnterGroupChat);
		}
	}
}

void ChatPlayer::OnRequestLeaveGroupChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_LEAVE_GROUP_CHAT oLeaveGroupChat;
	oLeaveGroupChat.Read(oNetStream);

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oLeaveGroupChat.dwGroupId)))
	{
		ChatGroup* pGroup = ChatServer::Instance()->GetChatGroupManager().GetChatGroup(oLeaveGroupChat.dwGroupId);
		if (!pGroup)
		{
			LogExe(LogLv_Error, "can't find group id : %d", oLeaveGroupChat.dwGroupId);
			return;
		}
		pGroup->OnLeaveGroupChat(m_szPyayerId);
	}
	else
	{
		stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT oC2CLeaveGroupChat;
		oC2CLeaveGroupChat.dwGroupId = oLeaveGroupChat.dwGroupId;
		oC2CLeaveGroupChat.szPlayerId = m_szPyayerId;

		ChatServerSessionManager& refServerSessionManager = ChatServer::Instance()->GetChatServerSessionManager();
		ChatServerSession* pChatServerSession = refServerSessionManager.GetChatServerSession(HashToIndex(oC2CLeaveGroupChat.dwGroupId));
		if (pChatServerSession)
		{
			pChatServerSession->OnLeaveGroupChat(oC2CLeaveGroupChat);
		}
	}
}

void ChatPlayer::OnLeaveGroupChatResult(stCHAT_ACK_PLAYER_LEAVE_GROUP_CHAT& refLeaveChatResult)
{
	CNetStream oStream(ENetStreamType_Write, g_pChatPlayerBuff, g_dwChatPlayerBuffLen);
	oStream.WriteInt(Protocol::CHAT_ACK_PLAYER_LEAVE_GROUP_CHAT);
	refLeaveChatResult.Write(oStream);
	m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}
