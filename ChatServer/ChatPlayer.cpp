#include "ChatPlayer.h"
#include "ChatServer.h"
#include "utility.h"

const static unsigned int g_dwChatPlayerBuffLen = 64 * 1024;
static char g_pChatPlayerBuf[g_dwChatPlayerBuffLen];

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

void ChatPlayer::OnPrivateChat(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_REQUEST_PRIVATE_CHAT oPLAYER_REQUEST_PRIVATE_CHAT;
	oPLAYER_REQUEST_PRIVATE_CHAT.Read(oNetStream);

	ChatServerSession* pChatServerSession = ChatServer::Instance()->GetChatServerSessionManager().GetChatServerSession(HashToIndex(oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId, IDLENTH));
	if (!pChatServerSession)
	{
		LogExe(LogLv_Critical, "can't find chat server session recver id : %s", oPLAYER_REQUEST_PRIVATE_CHAT.szRecverId);
	}
	// todo
}
