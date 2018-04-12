#include "LoginSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwLoginSessionBuffLen = 64 * 1024;
static char g_pLoginSessionBuf[g_dwLoginSessionBuffLen];

CLoginSession::CLoginSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::GameNotifyGameManagerInfo::descriptor(), &CLoginSession::OnGameNotifyGameManagerInfo);
}


CLoginSession::~CLoginSession()
{
}

void CLoginSession::OnConnect(void)
{
	//向对方发送本服务器信息
}

void CLoginSession::OnClose(void)
{

}

void CLoginSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CLoginSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

void CLoginSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

void CLoginSession::Init()
{
	m_dwServerId = 0;
}

bool CLoginSession::OnGameNotifyGameManagerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::GameNotifyGameManagerInfo* pMsg = dynamic_cast<GameProto::GameNotifyGameManagerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
CBinaryLoginSession::CBinaryLoginSession()
{
}

CBinaryLoginSession::~CBinaryLoginSession()
{
}

void CBinaryLoginSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CBinaryLoginSession * BinaryLoginSessionManager::CreateSession()
{
	CBinaryLoginSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool BinaryLoginSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryLoginSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryLoginSessionManager::Release(CBinaryLoginSession * pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

