#include "PlayerSession.h"
#include "netstream.h"
#include "msg_proto/web_game.pb.h"
//#include "gamedefine.h"

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

CPlayerSession::CPlayerSession()
	: m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(game_proto::PlayerRequestGameManagerInfo::descriptor(), &CPlayerSession::OnRequestGameManagerInfo);
}

CPlayerSession::~CPlayerSession()
{
}

void CPlayerSession::OnConnect(void)
{

}

void CPlayerSession::OnClose(void)
{

}

void CPlayerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CPlayerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	std::string szProtocolName;
	oStream.ReadString(szProtocolName);
	unsigned int dwProtoLen = oStream.GetDataLength();
	char* pData = oStream.ReadData(dwProtoLen);
	if (!m_oProtoDispatch.Dispatch(szProtocolName.c_str(),
		(const unsigned char*)pData, dwProtoLen, this, *this))
	{
		LogExe(LogLv_Debug, "%s proccess error", szProtocolName.c_str());
	}
}

void CPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

bool CPlayerSession::OnRequestGameManagerInfo(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
CWebSocketPlayerSession::CWebSocketPlayerSession()
{
}

CWebSocketPlayerSession::~CWebSocketPlayerSession()
{
}

void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
WebSocketPlayerSessionManager::WebSocketPlayerSessionManager()
{
}

WebSocketPlayerSessionManager::~WebSocketPlayerSessionManager()
{
}

FxSession * WebSocketPlayerSessionManager::CreateSession()
{
	FxSession* pSession = NULL;
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (m_oWebSocketPlayerSessions[i].GetConnection() == NULL)
		{
			m_oWebSocketPlayerSessions[i].Init((FxConnection*)0xFFFFFFFF);
			pSession = &m_oWebSocketPlayerSessions[i];
			break;
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_oLock.Lock();
	for (int i = 0; i < MAXSLAVESERVERNUM; ++i)
	{
		if (&m_oWebSocketPlayerSessions[i] == pSession)
		{
			m_oWebSocketPlayerSessions[i].Init(NULL);
			break;
		}
	}
	m_oLock.UnLock();
}
