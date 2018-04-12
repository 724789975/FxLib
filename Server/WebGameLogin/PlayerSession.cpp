#include "PlayerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"

#ifdef WIN32
#include <Windows.h>
#include <ShellAPI.h>
#else
#include<unistd.h>
#endif // WIN32

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

CPlayerSession::CPlayerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestLogin::descriptor(), &CPlayerSession::OnPlayerRequestLogin);
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

	GameServer::Instance()->GetPlayerManager().OnSessionClose(m_qwPlayerId);
	OnDestroy();

	FxSession::Init(NULL);
}

void CPlayerSession::Init()
{
	m_qwPlayerId = 0;
}

bool CPlayerSession::OnPlayerRequestLogin(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestLogin* pMsg = dynamic_cast<GameProto::PlayerRequestLogin*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameServer::Instance()->GetPlayerManager().OnPlayerLogin(this, *pMsg);

	CNetStream oWriteStream(ENetStreamType_Write, g_pPlayerSessionBuf, g_dwPlayerSessionBuffLen);
	GameProto::LoginAckPlayerLoginResult oResult;
	oResult.set_dw_result(1);
	oWriteStream.WriteString(oResult.GetTypeName());
	std::string szResult;
	oResult.SerializeToString(&szResult);
	oWriteStream.WriteData(szResult.c_str(), szResult.size());

	Send(g_pPlayerSessionBuf, g_dwPlayerSessionBuffLen - oWriteStream.GetDataLength());
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CWebSocketPlayerSession * WebSocketPlayerSessionManager::CreateSession()
{
	CWebSocketPlayerSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool WebSocketPlayerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

