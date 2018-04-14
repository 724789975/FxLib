#include "GameManagerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwGameManagerSessionBuffLen = 64 * 1024;
static char g_pGameManagerSessionBuf[g_dwGameManagerSessionBuffLen];

CGameManagerSession::CGameManagerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CGameManagerSession::OnServerInfo);
}


CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
	//向对方发送本服务器信息
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerid());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	//oInfo.set_dw_team_port((*it)->m_dwTeamPort);
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	CNetStream oWriteStream(ENetStreamType_Write, g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen);
	oWriteStream.WriteString(oInfo.GetTypeName());
	std::string szResult;
	oInfo.SerializeToString(&szResult);
	oWriteStream.WriteData(szResult.c_str(), szResult.size());
	Send(g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen - oWriteStream.GetDataLength());
}

void CGameManagerSession::OnClose(void)
{

}

void CGameManagerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CGameManagerSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

void CGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

void CGameManagerSession::Init()
{
	m_dwServerId = 0;
}

bool CGameManagerSession::OnServerInfo(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	return OnServerInfo(refSession, refMsg);
}

//////////////////////////////////////////////////////////////////////////
CBinaryGameManagerSession::CBinaryGameManagerSession()
{
	
}

CBinaryGameManagerSession::~CBinaryGameManagerSession()
{
}

bool CBinaryGameManagerSession::OnServerInfo(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::ServerInfo* pMsg = dynamic_cast<GameProto::ServerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	m_dwServerId = pMsg->dw_server_id();

	LogExe(LogLv_Debug, "server : %d connected, listen ip : %s login_port : %d, team_port : %d, game_manager_port : %d",
		pMsg->dw_server_id(), pMsg->sz_listen_ip().c_str(), pMsg->dw_login_port(),
		pMsg->dw_team_port(), pMsg->dw_game_server_manager_port());
	GameServer::Instance()->GetGameManagerSessionManager().GetGameManagerSessions()[m_dwServerId] = this;
	LogExe(LogLv_Debug, "server id : %d connected", m_dwServerId);
	return true;
}

void CBinaryGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CBinaryGameManagerSession * BinaryGameManagerSessionManager::CreateSession()
{
	CBinaryGameManagerSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool BinaryGameManagerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryGameManagerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryGameManagerSessionManager::Release(CBinaryGameManagerSession * pSession)
{
	m_mapGameManagerSessions.erase(pSession->GetServerId());
	m_poolSessions.ReleaseObj(pSession);
}

