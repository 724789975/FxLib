#include "ServerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwServerSessionBuffLen = 64 * 1024;
static char g_pServerSessionBuf[g_dwServerSessionBuffLen];

CServerSession::CServerSession()
	: m_oProtoDispatch(*this)
{
}


CServerSession::~CServerSession()
{
}

void CServerSession::OnConnect(void)
{

}

void CServerSession::OnClose(void)
{

}

void CServerSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CServerSession::OnRecv(const char* pBuf, unsigned int dwLen)
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

void CServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//void CServerSession::OnGameNotifyGameManagerInfo(const char* pBuf, UINT32 dwLen)
//{
//	CNetStream oStream(pBuf, dwLen);
//	stGAME_NOTIFY_GAME_MANAGER_INFO oGAME_NOTIFY_GAME_MANAGER_INFO;
//	oGAME_NOTIFY_GAME_MANAGER_INFO.Read(oStream);
//}

//////////////////////////////////////////////////////////////////////////

CWebSocketServerSession::CWebSocketServerSession()
{
}

CWebSocketServerSession::~CWebSocketServerSession()
{
}

void CWebSocketServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);

	GameServer::Instance()->GetWebSocketServerSessionManager().Release(this);
}

//////////////////////////////////////////////////////////////////////////
WebSocketServerSessionManager::WebSocketServerSessionManager()
{
	m_pCreated = NULL;
}

WebSocketServerSessionManager::~WebSocketServerSessionManager()
{
}

CWebSocketServerSession * WebSocketServerSessionManager::CreateSession()
{
	if (m_pCreated)
	{
		return NULL;
	}
	m_pCreated = &m_oWebSocketSlaveServerSession;
	return m_pCreated;
}

void WebSocketServerSessionManager::Release(FxSession * pSession)
{
	m_pCreated = NULL;
}
