#include "GMSession.h"
#include <sstream>
#include "ChatServerManager.h"
#include "json.h"

void GetInfo(GMSession* pSession, Json::Value& refjReq, Json::Value& refjAck)
{
	pSession->GetInfos(refjReq, refjAck);
}

GMSession::GMSession()
{
	m_mapOperate["get_info"] = GetInfo;
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

	Json::Value jReq;
	Json::Reader jReader;

	Json::Value jAck;
	Json::FastWriter jWriter;

	jAck["name"] = GetExeName();
	if (jReader.parse(pBuf, jReq))
	{
		if(jReq["opcode"].isString())
		{
			std::string szOpCode = jReq["opcode"].asString();
			if (m_mapOperate.find(szOpCode) == m_mapOperate.end())
			{
				jAck["ret"] = "opcode not found";
			}
			else
			{
				m_mapOperate[szOpCode](this, jReq, jAck);
			}
		}
		else
		{
			jAck["ret"] = "opcode error";
		}
	}
	else
	{
		jAck["ret"] = "request parse error";
	}

	std::string szAck = jWriter.write(jAck);

	Send(szAck.c_str(), szAck.size());
}

void GMSession::Release(void)
{
	OnDestroy();
}

void GMSession::GetInfos(Json::Value& refjReq, Json::Value& refjAck)
{
	ChatServerSessionManager& refSessionManager = ChatServerManager::Instance()->GetChatServerSessionManager();
	for (int i = 0; i < ChatConstant::g_dwChatServerNum; ++i)
	{
		if (!refSessionManager.GetChatServerSessions()[i].GetConnection())
		{
			continue;
		}
		Json::Value jInfo;
		jInfo["ChatIp"] = refSessionManager.GetChatServerSessions()[i].GetChatIp();
		jInfo["ChatPort"] = refSessionManager.GetChatServerSessions()[i].GetChatPort();
		jInfo["WebSocketChatPort"] = refSessionManager.GetChatServerSessions()[i].GetWebSocketChatPort();
		jInfo["ChatServerPort"] = refSessionManager.GetChatServerSessions()[i].GetChatServerPort();

		refjAck["info"].append(jInfo);
	}
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
