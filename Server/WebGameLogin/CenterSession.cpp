#include "CenterSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
#include "GameServer.h"
#include "LoginSession.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwCenterSessionBuffLen = 64 * 1024;
static char g_pCenterSessionBuf[g_dwCenterSessionBuffLen];

CCenterSession::CCenterSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CCenterSession::OnServerInfo);
}


CCenterSession::~CCenterSession()
{
}

void CCenterSession::OnConnect(void)
{
	GameProto::ServerInfo oInfo;
	oInfo.set_dw_server_id(GameServer::Instance()->GetServerid());
	//oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
	oInfo.set_dw_login_port(GameServer::Instance()->GetLoginPort());
	//oInfo.set_dw_team_port((*it)->m_dwTeamPort);
	oInfo.set_dw_game_server_manager_port(GameServer::Instance()->GetGameManagerPort());

	CNetStream oWriteStream(ENetStreamType_Write, g_pCenterSessionBuf, g_dwCenterSessionBuffLen);
	oWriteStream.WriteString(oInfo.GetTypeName());
	std::string szResult;
	oInfo.SerializeToString(&szResult);
	oWriteStream.WriteData(szResult.c_str(), szResult.size());
	Send(g_pCenterSessionBuf, g_dwCenterSessionBuffLen - oWriteStream.GetDataLength());
}

void CCenterSession::OnClose(void)
{
}

void CCenterSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CCenterSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

void CCenterSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

bool CCenterSession::OnServerInfo(CCenterSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::ServerInfo* pMsg = dynamic_cast<GameProto::ServerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	LogExe(LogLv_Debug, "server : %d info, listen ip : %s login_port : %d, team_port : %d, game_manager_port : %d",
		pMsg->dw_server_id(), pMsg->sz_listen_ip().c_str(), pMsg->dw_login_port(),
		pMsg->dw_team_port(), pMsg->dw_game_server_manager_port());
	
	if (pMsg->dw_server_id() / 10000 == GameProto::ST_Login)
	{
		CBinaryLoginSession* pLoginSession = GameServer::Instance()->GetLoginSessionManager().CreateSession();
		if (FxNetGetModule()->TcpConnect(pLoginSession, inet_addr(pMsg->sz_listen_ip().c_str()), pMsg->dw_login_port(), false) == INVALID_SOCKET)
		{
			LogExe(LogLv_Critical, "connect to login : %d faild", pMsg->dw_server_id());
		}
		else
		{
			LogExe(LogLv_Debug, "connect to login %s:%d", pMsg->sz_listen_ip().c_str(), pMsg->dw_server_id());
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
CBinaryCenterSession::CBinaryCenterSession()
{
}

CBinaryCenterSession::~CBinaryCenterSession()
{
}

void CBinaryCenterSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CBinaryCenterSession * BinaryCenterSessionManager::CreateSession()
{
	CBinaryCenterSession* pSession = m_poolSessions.FetchObj();
	return pSession;
}

bool BinaryCenterSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryCenterSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryCenterSessionManager::Release(CBinaryCenterSession * pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

