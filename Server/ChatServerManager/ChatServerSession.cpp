#include "ChatServerSession.h"
#include "netstream.h"
#include "ChatServerManager.h"
#include "chatdefine.h"

const static unsigned int g_dwChatServerSessionBuffLen = 64 * 1024;
static char g_pChatServerSessionBuf[g_dwChatServerSessionBuffLen];

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
	LogExe(LogLv_Debug, "remote ip : %s, remote port : %d", GetRemoteIPStr(), GetRemotePort());
}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(unsigned int dwErrorNo)
{

}

void ChatServerSession::OnRecv(const char* pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	Protocol::EChatProtocol eProrocol;
	oStream.ReadInt((int&)eProrocol);
	const char* pData = pBuf + sizeof(unsigned int);
	dwLen -= sizeof(unsigned int);

	switch (eProrocol)
	{
		case Protocol::CHAT_NOTIFY_CHAT_MANAGER_INFO:				OnChatServerInfo(pData, dwLen);	break;
		case Protocol::CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN:		OnChatLoginSign(pData, dwLen); break;
		case Protocol::CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN_GM:	OnChatLoginSignGM(pData, dwLen); break;
		default:	Assert(0);	break;
	}
}

void ChatServerSession::Release(void)
{
	OnDestroy();
}

void ChatServerSession::ChatLogin(std::string szServerId, std::string szPlayerId)
{
	stCHAT_MANAGER_NOTIFY_CHAT_LOGIN oCHAT_MANAGER_NOTIFY_CHAT_LOGIN;
	oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szPlayerId = szPlayerId;
	oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.szServerId = szServerId;

	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_MANAGER_NOTIFY_CHAT_LOGIN);
	oCHAT_MANAGER_NOTIFY_CHAT_LOGIN.Write(oStream);
	Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSession::ChatLoginByGM(std::string szPlayerId)
{
	stCHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM oCHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM;
	oCHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM.szPlayerId = szPlayerId;

	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM);
	oCHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM.Write(oStream);
	Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSession::OnChatServerInfo(const char* pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_SEND_CHAT_MANAGER_INFO oCHAT_SEND_CHAT_MANAGER_INFO;
	oCHAT_SEND_CHAT_MANAGER_INFO.Read(oStream);
	m_dwWebSocketChatPort = oCHAT_SEND_CHAT_MANAGER_INFO.m_dwWebSocketChatPort;
	m_dwChatPort = oCHAT_SEND_CHAT_MANAGER_INFO.m_dwChatPort;
	m_dwChatServerPort = oCHAT_SEND_CHAT_MANAGER_INFO.m_dwChatServerPort;
	m_szChatIp = oCHAT_SEND_CHAT_MANAGER_INFO.m_szChatIp;

	ChatServerManager::Instance()->GetChatServerSessionManager().OnChatServerInfo(this);
}

void ChatServerSession::OnChatLoginSign(const char* pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.Read(oStream);

	GameSession* pSession = ChatServerManager::Instance()->GetGameSessionManager().GetGameSession(oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szServerId);
	if (!pSession)
	{
		LogExe(LogLv_Critical, "error");
		return;
	}
	pSession->OnLoginSign(m_szChatIp, m_dwChatPort, m_dwWebSocketChatPort, oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szPlayerId, oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN.szSign);
}

void ChatServerSession::OnChatLoginSignGM(const char* pBuf, unsigned int dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM;
	oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.Read(oStream);

	ChatServerManager::Instance()->GetGMSessionManager().GetGMSession().OnLoginSign(m_szChatIp, m_dwChatPort, m_dwWebSocketChatPort,oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.szPlayerId, oCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM.szSign);
}

//----------------------------------------------------------------------
FxSession* ChatServerSessionManager::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	for (int i = 0; i < CHAT_SERVER_NUM; ++i)
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

void ChatServerSessionManager::CloseSessions()
{
	for (int i = 0; i < CHAT_SERVER_NUM; ++i)
	{
		m_oChatServerSessions[i].Close();
	}
	bool bClosed = true;
	do 
	{
		bClosed = true;
		FxNetGetModule()->Run(0xffffffff);
		FxSleep(10);
		for (int i = 0; i < CHAT_SERVER_NUM; ++i)
		{
			if (m_oChatServerSessions[i].GetConnection())
			{
				bClosed = false;
			}
		}
	} while (!bClosed);
}

void ChatServerSessionManager::Release(FxSession* pSession)
{

}

ChatServerSession* ChatServerSessionManager::GetChatServerSession(unsigned int dwHashIndex)
{
	if (m_mapSessionIpPort.find(dwHashIndex) == m_mapSessionIpPort.end())
	{
		return NULL;
	}
	return m_mapSessionIpPort[dwHashIndex];
}

void ChatServerSessionManager::OnChatServerInfo(ChatServerSession* pChatServerSession)
{
	stCHAT_MANAGER_NOTIFY_CHAT_INFO oCHAT_MANAGER_NOTIFY_CHAT_INFO;
	for (unsigned int i = 0; i < CHAT_SERVER_NUM; ++i)
	{
		if (pChatServerSession == m_oChatServerSessions + i)
		{
			// 同一个连接
			for (unsigned int j = 0; j < ChatConstant::g_dwHashGen; ++j)
			{
				if (j % CHAT_SERVER_NUM == i)
				{
					m_mapSessionIpPort[j] = pChatServerSession;
				}
			}
			oCHAT_MANAGER_NOTIFY_CHAT_INFO.dwHashIndex = i;
		}
		else
		{
			if (m_oChatServerSessions[i].m_dwChatPort && m_oChatServerSessions[i].m_dwChatServerPort)
			{
				stCHAT_MANAGER_NOTIFY_CHAT_INFO::stRemoteChatInfo oInfo;
				oInfo.dwIp = m_oChatServerSessions[i].GetRemoteIP();
				oInfo.dwPort = m_oChatServerSessions[i].m_dwChatServerPort;
				oInfo.dwHashIndex = i;
				oCHAT_MANAGER_NOTIFY_CHAT_INFO.vecRemoteInfo.push_back(oInfo);
			}
		}
	}
	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_MANAGER_NOTIFY_CHAT_INFO);
	oCHAT_MANAGER_NOTIFY_CHAT_INFO.Write(oStream);
	pChatServerSession->Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
}

void ChatServerSessionManager::BroadcastMsg(const Protocol::EChatType& eChatType, const std::string& szContent)
{
	stCHAT_MANAGER_NOTIFY_CHAT_BROADCAST oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST;

	oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.eChatType = eChatType;
	oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.szContent = szContent;
	CNetStream oStream(ENetStreamType_Write, g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen);
	oStream.WriteInt(Protocol::CHAT_MANAGER_NOTIFY_CHAT_BROADCAST);
	oCHAT_MANAGER_NOTIFY_CHAT_BROADCAST.Write(oStream);
	for (int i = 0; i < CHAT_SERVER_NUM; ++i)
	{
		m_oChatServerSessions[i].Send(g_pChatServerSessionBuf, g_dwChatServerSessionBuffLen - oStream.GetDataLength());
	}
}
