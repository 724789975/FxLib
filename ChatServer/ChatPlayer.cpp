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
		case Protocol::PLAYER_REQUEST_CREATE_CHAT_GROUP:	OnRequestCreateChatGroup(pData, dwLen);	break;
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
			oStream.WriteInt(Protocol::CHAT_SEND_PLAYER_PRIVATE_CHAT);
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
	oStream.WriteInt(Protocol::CHAT_SENF_CHAT_PRIVATE_CHAT);
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
	oStream.WriteInt(Protocol::CHAT_SEND_PLAYER_PRIVATE_CHAT);
	oCHAT_SEND_CHAT_PRIVATE_CHAT.Write(oStream);
	m_pSession->Send(g_pChatPlayerBuff, g_dwChatPlayerBuffLen - oStream.GetDataLength());
}

void ChatPlayer::OnRequestCreateChatGroup(const char* pBuf, UINT32 dwLen)
{

}
