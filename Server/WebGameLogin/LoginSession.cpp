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
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CLoginSession::OnServerInfo);
	m_oProtoDispatch.RegistFunction(GameProto::LoginNotifyLoginPlayerKick::descriptor(), &CLoginSession::OnLoginNotifyLoginPlayerKick);
	m_oProtoDispatch.RegistFunction(GameProto::LoginRequestLoginInviteTeam::descriptor(), &CLoginSession::OnLoginRequestLoginInviteTeam);
	m_oProtoDispatch.RegistFunction(GameProto::LoginAckLoginInviteTeam::descriptor(), &CLoginSession::OnLoginAckLoginInviteTeam);
	m_oProtoDispatch.RegistFunction(GameProto::LoginNotifyLoginPlayerRefuseEnterTeam::descriptor(), &CLoginSession::OnLoginNotifyLoginPlayerRefuseEnterTeam);
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
	oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	oInfo.set_dw_team_port(GameServer::Instance()->GetTeamPort());
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
}

void CLoginSession::OnClose(void)
{

}

void CLoginSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CLoginSession::OnRecv(const char* pBuf, unsigned int dwLen)
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

bool CLoginSession::OnLoginNotifyLoginPlayerKick(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginNotifyLoginPlayerKick* pMsg = dynamic_cast<GameProto::LoginNotifyLoginPlayerKick*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(pMsg->qw_player_id());
	if (pPlayer)
	{
		GameProto::LoginNotifyPlayerGameKick oKick;
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oKick, pBuf, dwBufLen);
		pPlayer->GetSession()->Send(pBuf, dwBufLen);
		pPlayer->GetSession()->Close();
	}

	return true;
}

bool CLoginSession::OnLoginRequestLoginInviteTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginRequestLoginInviteTeam* pMsg = dynamic_cast<GameProto::LoginRequestLoginInviteTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameProto::LoginAckLoginInviteTeam oResult;
	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(pMsg->qw_invitee_id());
	if (!pPlayer)
	{
		LogExe(LogLv_Critical, "can't find player : %llu", pMsg->qw_invitee_id());
		oResult.set_dw_result(GameProto::EC_CannotFindPlayer);
		oResult.set_qw_invite_id(pMsg->qw_invite_id());
		oResult.set_qw_invitee_id(pMsg->qw_invitee_id());
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}

	if (pPlayer->GetTeamId())
	{
		LogExe(LogLv_Critical, "already in team player : %llu", pMsg->qw_invitee_id());
		oResult.set_dw_result(GameProto::EC_AlreadyInTeam);
		oResult.set_qw_invite_id(pMsg->qw_invite_id());
		oResult.set_qw_invitee_id(pMsg->qw_invitee_id());
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}

	GameProto::LoginNotifyPlayerInviteTeam oNotify;
	oNotify.set_qw_player_id(pMsg->qw_invite_id());
	oNotify.set_qw_team_id(pMsg->qw_team_id());
	oNotify.set_dw_team_server_id(pMsg->dw_team_server_id());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oNotify, pBuf, dwBufLen);
	pPlayer->GetSession()->Send(pBuf, dwBufLen);

	return true;
}

bool CLoginSession::OnLoginAckLoginInviteTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginAckLoginInviteTeam* pMsg = dynamic_cast<GameProto::LoginAckLoginInviteTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(pMsg->qw_invite_id());
	if (!pPlayer)
	{
		LogExe(LogLv_Critical, "find player error : %llu", pMsg->qw_invite_id());
		return true;
	}

	GameProto::LoginAckPlayerInviteTeam oResult;
	oResult.set_dw_result(pMsg->dw_result());
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	pPlayer->GetSession()->Send(pBuf, dwBufLen);
	return true;
}

bool CLoginSession::OnLoginNotifyLoginPlayerRefuseEnterTeam(CLoginSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::LoginNotifyLoginPlayerRefuseEnterTeam* pMsg = dynamic_cast<GameProto::LoginNotifyLoginPlayerRefuseEnterTeam*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	Player* pPlayer = GameServer::Instance()->GetPlayerManager().GetPlayer(pMsg->qw_invite_id());
	if (!pPlayer)
	{
		LogExe(LogLv_Critical, "find player error : %llu", pMsg->qw_invite_id());
		return true;
	}
	GameProto::LoginNotifyPlayerRefuseEnterTeam oNoity;
	oNoity.set_qw_player_id(pMsg->qw_invitee_id());
	oNoity.set_sz_reason(pMsg->sz_reason());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oNoity, pBuf, dwBufLen);
	pPlayer->GetSession()->Send(pBuf, dwBufLen);
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

CBinaryLoginSession* BinaryLoginSessionManager::GetLoginSession(unsigned int dwServerId)
{
	if (m_mapLoginSessions.find(dwServerId) == m_mapLoginSessions.end())
	{
		return NULL;
	}

	return m_mapLoginSessions[dwServerId];
}

