#include "GMSession.h"
#include <sstream>
#include "ChatServerManager.h"
#include "chatdefine.h"
#include "ChatServerManager.h"
#include "utility.h"
#include "chatdefine.h"

const static unsigned int g_dwGMSessionBuffLen = 64 * 1024;
static char g_pGMSessionBuf[g_dwGMSessionBuffLen];

GMSession::GMSession()
{
	m_mapOperate["get_info"] = &GMSession::GetInfo;
	m_mapOperate["broadcast"] = &GMSession::Broadcast;
	m_mapOperate["login"] = &GMSession::LoginTest;
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
		if (jReq["opcode"].isString())
		{
			std::string szOpCode = jReq["opcode"].asString();
			if (m_mapOperate.find(szOpCode) == m_mapOperate.end())
			{
				jAck["ret"] = "opcode not found";
			}
			else
			{
				if(!(this->*(m_mapOperate[szOpCode]))(jReq, jAck))
				{
					return;
				}
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

bool GMSession::GetInfo(Json::Value& refjReq, Json::Value& refjAck)
{
	ChatServerSessionManager& refSessionManager = ChatServerManager::Instance()->GetChatServerSessionManager();
	for (int i = 0; i < CHAT_SERVER_NUM; ++i)
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
	return true;
}

bool GMSession::Broadcast(Json::Value& refjReq, Json::Value& refjAck)
{
	if (!refjReq["chat_type"].isInt())
	{
		refjAck["ret"] = "error chat type";
		return true;
	}
	Protocol::EChatType eChatType = (Protocol::EChatType)refjReq["chat_type"].asInt();

	if (!refjReq["content"].isString())
	{
		refjAck["ret"] = "error content";
		return true;
	}
	std::string szContent = refjReq["content"].asString();

	ChatServerManager::Instance()->GetChatServerSessionManager().BroadcastMsg(eChatType, szContent);

	refjAck["ret"] = "ok";

	return true;
}

bool GMSession::LoginTest(Json::Value& refjReq, Json::Value& refjAck)
{
	if (!refjReq["player_id"].isString())
	{
		refjAck["ret"] = "error player id";
		return true;
	}

	std::string szPlayerId = refjReq["player_id"].asString();
	if (szPlayerId.size() > IDLENTH)
	{
		refjAck["ret"] = "id too long";
		return true;
	}

	UINT32 dwHashIndex =  HashToIndex(szPlayerId.c_str(), szPlayerId.size());
	ChatServerSession* pServerSession = ChatServerManager::Instance()->GetChatServerSessionManager().GetChatServerSession(dwHashIndex);
	if (!pServerSession)
	{
		refjAck["ret"] = "error hash index";
		return true;
	}

	pServerSession->ChatLoginByGM(szPlayerId);
	return false;
}

void GMSession::OnLoginSign(std::string szChatIp, unsigned int dwChatPort, unsigned int dwWebSocketChatPort, std::string szPlayerId, std::string szSign)
{
	Json::Value jAck;
	Json::FastWriter jWriter;

	jAck["player_id"] = szPlayerId;
	jAck["sign"] = szSign;
	jAck["chat_ip"] = szChatIp;
	jAck["chat_port"] = dwChatPort;
	jAck["web_scoket_chat_port"] = dwWebSocketChatPort;

	std::string szAck = jWriter.write(jAck);
	Send(szAck.c_str(), szAck.size());
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
