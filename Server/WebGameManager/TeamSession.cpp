#include "TeamSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

#include "gflags/gflags.h"

DECLARE_string(redis_ip);
DECLARE_string(redis_pw);
DECLARE_uint32(redis_port);

const static unsigned int g_dwTeamSessionBuffLen = 64 * 1024;
static char g_pTeamSessionBuf[g_dwTeamSessionBuffLen];

bool StartProccess(unsigned long long qwTeamId, unsigned int dwTeamServerId, char* szRoles)
{
#ifdef WIN32
	SHELLEXECUTEINFO shell = { sizeof(shell) };
	shell.fMask = SEE_MASK_NOCLOSEPROCESS;
	shell.lpVerb = "open";
	shell.lpFile = "WebGame.exe";
	char szBuffer[512] = { 0 };
	GetExePath();
	sprintf(szBuffer, "--game_manager_ip %s --game_manager_port %d --team_id %llu --team_server_id %d --roles %s --redis_ip %s --redis_pw %s --redis_port %d",
		"127.0.0.1", GameServer::Instance()->GetServerListenPort(), qwTeamId, dwTeamServerId, szRoles, FLAGS_redis_ip.c_str(), FLAGS_redis_pw.c_str(), FLAGS_redis_port);

	LogExe(LogLv_Debug, "%s", szBuffer);
	shell.lpParameters = szBuffer;
	shell.lpDirectory = GetExePath();
	shell.nShow = SW_SHOWNORMAL;
	BOOL ret = ShellExecuteEx(&shell);
	return ret == TRUE;
#else
	char szExePath[512] = { 0 };
	sprintf(szExePath, "%s/WebGame", GetExePath());
	char szServerIp[32] = { 0 };
	sprintf(szServerIp, "%s", "127.0.0.1");
	char szServerPort[8] = { 0 };
	sprintf(szServerPort, "%d", GameServer::Instance()->GetServerListenPort());
	char szTeamId[32] = { 0 };
	sprintf(szTeamId, "%llu", qwTeamId);
	char szTeamServerId[32] = { 0 };
	sprintf(szTeamServerId, "%d", dwTeamServerId);
	char szRedisPort[32] = { 0 };
	sprintf(szRedisPort, "%d", FLAGS_redis_port);
	char szRedisIp[32] = { 0 };
	sprintf(szRedisIp, "%s", FLAGS_redis_ip.c_str());
	char szRedisPw[32] = { 0 };
	sprintf(szRedisPw, "%s", FLAGS_redis_pw.c_str());
	char *arg[] = { szExePath, "--game_manager_ip", szServerIp,
		"--game_manager_port", szServerPort, "--team_id", szTeamId,
		"--redis_ip", szRedisIp, "--redis_pw", szRedisPw, "--redis_port", szRedisPort,
		"--team_server_id", szTeamServerId, "--roles", szRoles, 0 };
	LogExe(LogLv_Debug, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10], arg[11], arg[12], arg[13], arg[14], arg[15], arg[16]);
	int pid = vfork();
	if (pid < 0)
	{
		LogExe(LogLv_Critical, "start game fail");
		return false;
	}

	if (pid == 0)
	{
		execv(arg[0], arg);
		_exit(0);
	}
	else
	{
		return true;
	}
	return false;
#endif // WIN32
}

CTeamSession::CTeamSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CTeamSession::OnServerInfo);
	m_oProtoDispatch.RegistFunction(GameProto::TeamRequestGameManagerGameStart::descriptor(), &CTeamSession::OnTeamRequestGameManagerGameStart);
}

CTeamSession::~CTeamSession()
{
}

void CTeamSession::OnConnect(void)
{
	//向对方发送本服务器信息
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerId());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	//oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	//oInfo.set_dw_team_port(GameServer::Instance()->GetTeamPort());
	//oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
}

void CTeamSession::OnClose(void)
{
}

void CTeamSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CTeamSession::OnRecv(const char* pBuf, unsigned int dwLen)
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

void CTeamSession::Init()
{
	m_dwServerId = 0;
}

bool CTeamSession::OnServerInfo(CTeamSession& refSession, google::protobuf::Message& refMsg)
{
	return OnServerInfo(refSession, refMsg);
}

bool CTeamSession::OnTeamRequestGameManagerGameStart(CTeamSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::TeamRequestGameManagerGameStart* pMsg = dynamic_cast<GameProto::TeamRequestGameManagerGameStart*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	char szRoles[MAXCLIENTNUM * 32] = {0};
	unsigned long long dwLen = 0;
	dwLen += sprintf(szRoles + dwLen, "[");
	for (int i = 0; i < pMsg->qw_player_ids_size(); ++i)
	{
		dwLen += sprintf(szRoles + dwLen, "%llu,", pMsg->qw_player_ids(i));
	}
	szRoles[dwLen - 1] = ']';
	StartProccess(pMsg->qw_team_id(), m_dwServerId, szRoles);
	return true;
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

	//FxSession::Init(NULL);
	GameServer::Instance()->GetTeamSessionManager().Release(this);
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

