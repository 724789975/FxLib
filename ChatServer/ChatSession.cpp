#include "ChatSession.h"
#include "ChatServer.h"
#include "utility.h"
#include "chatdefine.h"

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
	memset(m_szId, 0, IDLENTH);
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

void ChatSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
	Reset();

	ChatServer::Instance()->GetChatSessionManager().Release(this);
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
		case Protocol::PLAYER_LOGIN:	OnLogin(pData, dwLen);	break;
		default: {LogExe(LogLv_Critical, "error protocot : %d", (unsigned int)eProrocol); Close(); }	break;
	}
}

void ChatSession::OnLogin(const char* pBuf, UINT32 dwLen)
{
	CNetStream oNetStream(pBuf, dwLen);
	stPLAYER_LOGIN oPLAYER_LOGIN;
	oPLAYER_LOGIN.Read(oNetStream);

	if (ChatServer::Instance()->CheckHashIndex(HashToIndex(oPLAYER_LOGIN.szId, IDLENTH)))
	{
		ChatServer::Instance()->GetChatPlayerManager().OnPlayerLogin(oPLAYER_LOGIN.szId, this);
	}
}

FxSession* ChatSessionManager::CreateSession()
{
	return m_poolSessions.FetchObj();
}

bool ChatSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void ChatSessionManager::Release(FxSession* pSession)
{
	Assert(0);
	//m_poolSessions.ReleaseObj()
}

void ChatSessionManager::Release(ChatSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}
