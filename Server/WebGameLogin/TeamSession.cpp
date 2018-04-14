#include "TeamSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwTeamSessionBuffLen = 64 * 1024;
static char g_pTeamSessionBuf[g_dwTeamSessionBuffLen];

CTeamSession::CTeamSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CTeamSession::OnServerInfo);
}


CTeamSession::~CTeamSession()
{
}

void CTeamSession::OnConnect(void)
{
	//向对方发送本服务器信息
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerid());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	oInfo.set_dw_team_port(GameServer::Instance()->GetTeamPort());
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	CNetStream oWriteStream(ENetStreamType_Write, g_pTeamSessionBuf, g_dwTeamSessionBuffLen);
	oWriteStream.WriteString(oInfo.GetTypeName());
	std::string szResult;
	oInfo.SerializeToString(&szResult);
	oWriteStream.WriteData(szResult.c_str(), szResult.size());
	Send(g_pTeamSessionBuf, g_dwTeamSessionBuffLen - oWriteStream.GetDataLength());
}

void CTeamSession::OnClose(void)
{

}

void CTeamSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CTeamSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

void CTeamSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

void CTeamSession::Init()
{
	m_dwServerId = 0;
}

bool CTeamSession::OnServerInfo(CTeamSession& refSession, google::protobuf::Message& refMsg)
{
	return OnServerInfo(refSession, refMsg);
}

//////////////////////////////////////////////////////////////////////////
CBinaryTeamSession::CBinaryTeamSession()
{
	
}

CBinaryTeamSession::~CBinaryTeamSession()
{
}

bool CBinaryTeamSession::OnServerInfo(CTeamSession& refSession, google::protobuf::Message& refMsg)
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
	GameServer::Instance()->GetTeamSessionManager().GetTeamSessions()[m_dwServerId] = this;
	LogExe(LogLv_Debug, "server id : %d connected", m_dwServerId);
	return true;
}

void CBinaryTeamSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CBinaryTeamSession * BinaryTeamSessionManager::CreateSession()
{
	CBinaryTeamSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool BinaryTeamSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryTeamSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryTeamSessionManager::Release(CBinaryTeamSession * pSession)
{
	m_mapTeamSessions.erase(pSession->GetServerId());
	m_poolSessions.ReleaseObj(pSession);
}

