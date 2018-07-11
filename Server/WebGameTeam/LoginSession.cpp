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
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamKickPlayer::descriptor(), &CLoginSession::OnLoginRequestTeamKickPlayer);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamGameStart::descriptor(), &CLoginSession::OnLoginRequestTeamGameStart);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamEnterTeam::descriptor(), &CLoginSession::OnLoginRequestTeamEnterTeam);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestTeamPlayerLeave::descriptor(), &CLoginSession::OnLoginRequestTeamPlayerLeave);
	
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
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

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
	LogExe(LogLv_Debug, "server id : %d, protocol name : %s", m_dwServerId, szProtocolName.c_str());
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
	Assert(pTeamRoleData);

	pTeamRoleData->set_dw_server_id(m_dwServerId);
	GameProto::TeamAckLoginMakeTeam oTeamAckLoginMakeTeam;
	oTeamAckLoginMakeTeam.set_dw_result(0);
	oTeamAckLoginMakeTeam.set_qw_player_id(pMsg->role_data().qw_player_id());
	oTeamAckLoginMakeTeam.set_qw_team_id(pMsg->qw_team_id());
	oTeamAckLoginMakeTeam.set_dw_slot_id(pTeamRoleData->dw_slot_id());

	LogExe(LogLv_Debug, "player : %llu maketeam id : %llu",
		oTeamAckLoginMakeTeam.qw_player_id(), oTeamAckLoginMakeTeam.qw_team_id());

	//notify
	refTeam.NotifyPlayer();

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

	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	GameProto::TeamAckLoginChangeSlot oChangeSlot;
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	oChangeSlot.set_qw_player_id(pMsg->qw_player_id());
	if (pTeam == NULL)
	{
		oChangeSlot.set_dw_result(GameProto::EC_NoTeamId);
		LogExe(LogLv_Critical, "player : %llu want leave team : %llu no team id", pMsg->qw_player_id(), pMsg->qw_team_id());

		ProtoUtility::MakeProtoSendBuffer(oChangeSlot, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	oChangeSlot.set_dw_result(pTeam->ChangeSlot(pMsg->qw_player_id(), pMsg->dw_slot_id()));
	ProtoUtility::MakeProtoSendBuffer(oChangeSlot, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
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

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oKickPlayer, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
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

	if (pTeam->GetTeamNum() == 0)
	{
		GameServer::Instance()->GetTeamManager().ReleaseTeam(pMsg->qw_team_id());
	}
	else
	{
		pTeam->NotifyPlayer();
	}
	return true;
}

bool CLoginSession::OnLoginRequestTeamGameStart(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamGameStart* pMsg = dynamic_cast<GameProto::LoginRequestTeamGameStart*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameProto::TeamAckLoginGameStart oResult;
	oResult.set_qw_player_id(pMsg->qw_player_id());
	oResult.set_qw_team_id(pMsg->qw_team_id());
	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	if (pTeam == NULL)
	{
		LogExe(LogLv_Critical, "can't find team id : %llu, player id : %llu", pMsg->qw_team_id(), pMsg->qw_player_id());
		oResult.set_dw_result(GameProto::EC_NoTeamId);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	if (pTeam->GetLeaderId() != pMsg->qw_player_id())
	{
		LogExe(LogLv_Info, "player not leader team id : %llu, player id : %llu", pMsg->qw_team_id(), pMsg->qw_player_id());
		oResult.set_dw_result(GameProto::EC_NotLeader);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	if (pTeam->GetState() == CTeam::ETS_StartGame)
	{
		LogExe(LogLv_Info, "team already has start id : %llu, player id : %llu", pMsg->qw_team_id(), pMsg->qw_player_id());
		oResult.set_dw_result(GameProto::EC_TeamHasStart);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	std::map<unsigned int, CBinaryGameManagerSession*>& refSessions = GameServer::Instance()->GetGameManagerSessionManager().GetGameManagerSessions();
	if (refSessions.size() == 0)
	{
		LogExe(LogLv_Info, "no game manager server team id : %llu, player id : %llu", pMsg->qw_team_id(), pMsg->qw_player_id());
		oResult.set_dw_result(GameProto::EC_NoGameManagerServer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	pTeam->SetState(CTeam::ETS_StartGame);
	static unsigned int s_dwTeamIndex;
	unsigned int dwTeamIndex = ++s_dwTeamIndex % refSessions.size();
	std::map<unsigned int, CBinaryGameManagerSession*>::iterator it = refSessions.begin();
	for (unsigned int i = 0; i < dwTeamIndex; ++i)
	{
		++it;
	}
	GameProto::TeamRequestGameManagerGameStart oRequest;
	oRequest.set_qw_team_id(pMsg->qw_team_id());
	for (UINT32 i = 0; i < MAXCLIENTNUM; ++i)
	{
		oRequest.add_qw_player_ids(pTeam->GetTeam()[i]);
	}

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oRequest, pBuf, dwBufLen);
	LogExe(LogLv_Debug, "team id : %llu, player id : %llu request start game", pMsg->qw_team_id(), pMsg->qw_player_id());
	(it->second)->Send(pBuf, dwBufLen);

	return true;
}

bool CLoginSession::OnLoginRequestTeamEnterTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamEnterTeam* pMsg = dynamic_cast<GameProto::LoginRequestTeamEnterTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	const GameProto::RoleData& refRoleData = pMsg->role_data();
	GameProto::TeamAckLoginEnterTeam oResult;
	oResult.set_qw_player_id(refRoleData.qw_player_id());
	oResult.set_qw_team_id(pMsg->qw_team_id());
	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	if (pTeam == NULL)
	{
		LogExe(LogLv_Critical, "cann't find team id : %llu, player id : %llu", pMsg->qw_team_id(), refRoleData.qw_player_id());
		oResult.set_dw_result(GameProto::EC_NoTeamId);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}

	if (!pTeam->InsertIntoTeam(*(pMsg->mutable_role_data())))
	{
		LogExe(LogLv_Critical, "cann't enter team id : %llu, player id : %llu", pMsg->qw_team_id(), refRoleData.qw_player_id());
		oResult.set_dw_result(GameProto::EC_FailIntoTeam);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	GameProto::TeamRoleData* pTeamRoleData = pTeam->GetTeamRoleData(refRoleData.qw_player_id());
	if (pTeamRoleData == NULL)
	{
		LogExe(LogLv_Critical, "get team role data error player id : %llu", refRoleData.qw_player_id());
		return false;
	}
	pTeamRoleData->set_dw_server_id(m_dwServerId);

	//notify
	pTeam->NotifyPlayer();

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	return true;
}

bool CLoginSession::OnLoginRequestTeamPlayerLeave(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestTeamPlayerLeave* pMsg = dynamic_cast<GameProto::LoginRequestTeamPlayerLeave*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	CTeam* pTeam = GameServer::Instance()->GetTeamManager().GetTeam(pMsg->qw_team_id());
	GameProto::TeamAckLoginPlayerLeave oPlayerLeave;
	oPlayerLeave.set_qw_player_id(pMsg->qw_player_id());
	if (pTeam == NULL)
	{
		oPlayerLeave.set_dw_result(GameProto::EC_NoTeamId);
		LogExe(LogLv_Critical, "player : %llu want leave team : %llu no team id", pMsg->qw_player_id(), pMsg->qw_team_id());

		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oPlayerLeave, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	else
	{
		if (!pTeam->KickPlayer(pMsg->qw_player_id()))
		{
			oPlayerLeave.set_dw_result(GameProto::EC_NoTeamId);
			LogExe(LogLv_Critical, "player : %llu want leave team : %llu kick fail", pMsg->qw_player_id(), pMsg->qw_team_id());
		}
	}
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oPlayerLeave, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	if (pTeam->GetTeamNum() == 0)
	{
		GameServer::Instance()->GetTeamManager().ReleaseTeam(pMsg->qw_team_id());
	}
	else
	{
		pTeam->NotifyPlayer();
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
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	//FxSession::Init(NULL);
	GameServer::Instance()->GetLoginSessionManager().Release(this);
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

CBinaryLoginSession* BinaryLoginSessionManager::GetLoginSession(UINT32 dwServerId)
{
	if (m_mapLoginSessions.find(dwServerId) == m_mapLoginSessions.end())
	{
		return NULL;
	}
	return m_mapLoginSessions[dwServerId];
}

