#include "GMSession.h"
#include <sstream>
#include "ChatServerManager.h"

GMSession::GMSession()
{
}


GMSession::~GMSession()
{
}

void GMSession::OnConnect(void)
{
	LogExe(LogLv_Debug, "remote ip : %s, remote port : %d", GetRemoteIPStr(), GetRemotePort());
}

void GMSession::OnClose(void)
{

}

void GMSession::OnError(UINT32 dwErrorNo)
{

}

void GMSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	const_cast<char*>(pBuf)[dwLen] = 0;
	LogExe(LogLv_Debug, "%s", pBuf);
	std::stringstream sStream;
	sStream << GetExeName() << ", connections \n";
	ChatServerSessionManager& refSessionManager = ChatServerManager::Instance()->GetChatServerSessionManager();
	for (int i = 0; i < ChatConstant::g_dwChatServerNum; ++i)
	{
		if (!refSessionManager.GetChatServerSessions()[i].GetConnection())
		{
			continue;
		}
		sStream << i << ", ChatIp : " << refSessionManager.GetChatServerSessions()[i].GetChatIp();
		sStream << ", ChatPort : " << refSessionManager.GetChatServerSessions()[i].GetChatPort();
		sStream << ", WebSocketChatPort : " << refSessionManager.GetChatServerSessions()[i].GetWebSocketChatPort();
		sStream << ", ChatServerPort : " << refSessionManager.GetChatServerSessions()[i].GetChatServerPort() << "\n";
	}

	Send(sStream.str().c_str(), sStream.str().size());
}

void GMSession::Release(void)
{
	OnDestroy();
}

FxSession* GMSessionManager::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	if (m_oGMSession.GetConnection() == NULL)
	{
		m_oGMSession.Init((FxConnection*)0xFFFFFFFF);
		pSession = &m_oGMSession;
	}
	m_oLock.UnLock();
	return pSession;
}

void GMSessionManager::Release(FxSession* pSession)
{

}
