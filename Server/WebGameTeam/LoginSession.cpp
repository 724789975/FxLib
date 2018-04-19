#include "LoginSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"
#include "fxredis.h"

const static unsigned int g_dwLoginSessionBuffLen = 64 * 1024;
static char g_pLoginSessionBuf[g_dwLoginSessionBuffLen];

CLoginSession::CLoginSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CLoginSession::OnServerInfo);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamMakeTeam::descriptor(), &CLoginSession::OnLoginRequestTeamMakeTeam);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamInviteTeam::descriptor(), &CLoginSession::OnLoginRequestTeamInviteTeam);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamChangeSlot::descriptor(), &CLoginSession::OnLoginRequestTeamChangeSlot);
}

CLoginSession::~CLoginSession()
{
}

void CLoginSession::OnConnect(void)
{
	//向对方发送本服务器信息
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerId());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	//oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	//oInfo.set_dw_team_port((*it)->m_dwTeamPort);
	//oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
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

void CLoginSession::Init()
{
	m_dwServerId = 0;
}

bool CLoginSession::OnServerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	return OnServerInfo(refSession, refMsg);
}

bool CLoginSession::OnLoginRequestTeamMakeTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamMakeTeam* pMsg = dynamic_cast<GameProto::LoginRequestTeamMakeTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	CTeam& refTeam = GameServer::Instance()->GetTeamManager().CreateTeam(pMsg->qw_team_id());
	refTeam.InsertIntoTeam(pMsg->role_data());
	GameProto::TeamRoleData* pTeamRoleData = refTeam.GetTeamRoleData(pMsg->role_data().qw_player_id());
	pTeamRoleData->set_dw_server_id(m_dwServerId);
	if (pTeamRoleData == NULL)
	{
		Assert(0);
	}

	GameProto::TeamAckLoginMakeTeam oTeamAckLoginMakeTeam;
	oTeamAckLoginMakeTeam.set_dw_result(0);
	oTeamAckLoginMakeTeam.set_qw_player_id(pMsg->role_data().qw_player_id());
	oTeamAckLoginMakeTeam.set_qw_team_id(pMsg->qw_team_id());
	oTeamAckLoginMakeTeam.set_dw_slot_id(pTeamRoleData->dw_slot_id());

	LogExe(LogLv_Debug, "player : %llu maketeam id : %llu",
		oTeamAckLoginMakeTeam.qw_player_id(), oTeamAckLoginMakeTeam.qw_team_id());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oTeamAckLoginMakeTeam, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

bool CLoginSession::OnLoginRequestTeamInviteTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamInviteTeam* pMsg = dynamic_cast<GameProto::LoginRequestTeamInviteTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	return true;
}

bool CLoginSession::OnLoginRequestTeamChangeSlot(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamChangeSlot* pMsg = dynamic_cast<GameProto::LoginRequestTeamChangeSlot*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	return true;
}

bool CLoginSession::OnLoginRequestTeamKickPlayer(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamKickPlayer* pMsg = dynamic_cast<GameProto::LoginRequestTeamKickPlayer*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	GameProto::TeamAckLoginKickPlayer oKickPlayer;
	oKickPlayer.set_qw_player_id(pMsg->qw_player_id());
	if (pTeam == NULL)
	{
		oKickPlayer.set_dw_result(GameProto::EC_NoTeamId);
		LogExe(LogLv_Critical, "player : %llu want leave team : %llu no team id", pMsg->qw_player_id(), pMsg->qw_team_id());
	}
	else
	{
		if (!pTeam->KickPlayer(pMsg->qw_player_id()))
		{
			oKickPlayer.set_dw_result(GameProto::EC_NoTeamId);
			LogExe(LogLv_Critical, "player : %llu want leave team : %llu kick fail", pMsg->qw_player_id(), pMsg->qw_team_id());
		}
	}
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oKickPlayer, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CBinaryLoginSession::CBinaryLoginSession()
{
}

CBinaryLoginSession::~CBinaryLoginSession()
{
}

bool CBinaryLoginSession::OnServerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg)
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
	GameServer::Instance()->GetLoginSessionManager().GetLoginSessions()[m_dwServerId] = this;
	LogExe(LogLv_Debug, "server id : %d connected", m_dwServerId);
	return true;
}

void CBinaryLoginSession::Release(void)
{
	//LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	//OnDestroy();

	//FxSession::Init(NULL);
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
	m_mapLoginSessions.erase(pSession->GetServerId());
	m_poolSessions.ReleaseObj(pSession);
}

