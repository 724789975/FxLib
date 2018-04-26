#include "GameManagerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwGameManagerSessionBuffLen = 64 * 1024;
static char g_pGameManagerSessionBuf[g_dwGameManagerSessionBuffLen];

CGameManagerSession::CGameManagerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CGameManagerSession::OnServerInfo);
	m_oProtoDispatch.RegistFunction(GameProto::GameManagerAckTeamGameStart::descriptor(), &CGameManagerSession::OnGameManagerAckTeamGameStart);
}


CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
	//向对方发送本服务器信息
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerId());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	//oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	//oInfo.set_dw_team_port(GameServer::Instance()->GetTeamPort());
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
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
	LogExe(LogLv_Debug, "server id : %d, protocol name : %s", m_dwServerId, szProtocolName.c_str());
	unsigned int dwProtoLen = oStream.GetDataLength();
	char* pData = oStream.ReadData(dwProtoLen);
	if (!m_oProtoDispatch.Dispatch(szProtocolName.c_str(),
		(const unsigned char*)pData, dwProtoLen, this, *this))
	{
		LogExe(LogLv_Debug, "%s proccess error", szProtocolName.c_str());
	}
}

void CGameManagerSession::Init()
{
	m_dwServerId = 0;
}

bool CGameManagerSession::OnServerInfo(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	return OnServerInfo(refSession, refMsg);
}

bool CGameManagerSession::OnGameManagerAckTeamGameStart(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::GameManagerAckTeamGameStart* pMsg = dynamic_cast<GameProto::GameManagerAckTeamGameStart*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	if (pTeam == NULL)
	{
		LogExe(LogLv_Critical, "get team error id : %llu", pMsg->qw_team_id());
		return true;
	}
	if (pMsg->dw_result() != 0)
	{
		LogExe(LogLv_Critical, "get team start error : %d, team id : %llu", pMsg->dw_result(), pMsg->qw_team_id());
		pTeam->SetState(CTeam::ETS_Idle);
		return true;
	}
	GameProto::TeamAckLoginGameStart oResult;
	oResult.set_dw_result(pMsg->dw_result());
	oResult.set_dw_player_port(pMsg->dw_player_port());
	oResult.set_dw_server_port(pMsg->dw_server_port());
	oResult.set_dw_slave_server_port(pMsg->dw_slave_server_port());
	oResult.set_sz_listen_ip(pMsg->sz_listen_ip());
	oResult.set_qw_team_id(pMsg->qw_team_id());
	std::map<UINT64, GameProto::TeamRoleData>& refRoles = pTeam->GetTeamRoles();
	std::map<unsigned int, CBinaryLoginSession*>& refLoginSessions = GameServer::Instance()->GetLoginSessionManager().GetLoginSessions();
	for (std::map<UINT64, GameProto::TeamRoleData>::iterator it = refRoles.begin();
		it != refRoles.end(); ++it)
	{
		std::map<unsigned int, CBinaryLoginSession*>::iterator it1 = refLoginSessions.find((it->second).dw_server_id());
		if (it1 == refLoginSessions.end())
		{
			continue;
		}
		oResult.set_qw_player_id(it->first);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		it1->second->Send(pBuf, dwBufLen);
	}

	GameServer::Instance()->GetTeamManager().ReleaseTeam(pMsg->qw_team_id());
	return true;
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

	//FxSession::Init(NULL);
	GameServer::Instance()->GetGameManagerSessionManager().Release(this);
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

