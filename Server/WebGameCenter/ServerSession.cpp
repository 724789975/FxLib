#include "ServerSession.h"
#include "netstream.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwServerSessionBuffLen = 64 * 1024;
static char g_pServerSessionBuf[g_dwServerSessionBuffLen];

CServerSession::CServerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::GameNotifyGameManagerInfo::descriptor(), &CServerSession::OnGameNotifyGameManagerInfo);
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

void CServerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

	FxSession::Init(NULL);
}

void CServerSession::Init()
{
	m_dwServerId = 0;
	//m_szTeamServerIp = "";
	m_dwTeamPort = 0;
	//m_szGameServerManagerIp = "";
	m_dwGameServerManagerPort = 0;
	//m_szLoginServerIp = "";
	m_dwLoginPort = 0;
}

bool CServerSession::OnGameNotifyGameManagerInfo(CServerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::GameNotifyGameManagerInfo* pMsg = dynamic_cast<GameProto::GameNotifyGameManagerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	return true;
}

bool CServerSession::OnServerInfo(CServerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::ServerInfo* pMsg = dynamic_cast<GameProto::ServerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}
	m_dwServerId = pMsg->dw_server_id();
	m_dwLoginPort = pMsg->dw_login_port();
	m_dwTeamPort = pMsg->dw_team_port();
	m_dwGameServerManagerPort = pMsg->dw_game_server_manager_port();

	pMsg->set_sz_listen_ip(GetRemoteIPStr());

	std::set<CBinaryServerSession*>& refSessions = GameServer::Instance()->GetBinaryServerSessionManager().GetSessions();
	for (std::set<CBinaryServerSession*>::iterator it = refSessions.begin(); it != refSessions.end(); ++it)
	{
		{
			CNetStream oWriteStream(ENetStreamType_Write, g_pServerSessionBuf, g_dwServerSessionBuffLen);
			oWriteStream.WriteString(pMsg->GetTypeName());
			std::string szResult;
			pMsg->SerializeToString(&szResult);
			oWriteStream.WriteData(szResult.c_str(), szResult.size());

			(*it)->Send(g_pServerSessionBuf, g_dwServerSessionBuffLen - oWriteStream.GetDataLength());
		}
		{
			GameProto::ServerInfo oInfo;
			oInfo.set_dw_server_id((*it)->m_dwServerId);
			oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
			oInfo.set_dw_login_port((*it)->m_dwLoginPort);
			oInfo.set_dw_team_port((*it)->m_dwTeamPort);
			oInfo.set_dw_game_server_manager_port((*it)->m_dwGameServerManagerPort);

			CNetStream oWriteStream(ENetStreamType_Write, g_pServerSessionBuf, g_dwServerSessionBuffLen);
			oWriteStream.WriteString(pMsg->GetTypeName());
			std::string szResult;
			pMsg->SerializeToString(&szResult);
			oWriteStream.WriteData(szResult.c_str(), szResult.size());
			Send(g_pServerSessionBuf, g_dwServerSessionBuffLen - oWriteStream.GetDataLength());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CBinaryServerSession::CBinaryServerSession()
{
}

CBinaryServerSession::~CBinaryServerSession()
{
}

void CBinaryServerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	FxSession::Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
FxSession * BinaryServerSessionManager::CreateSession()
{
	CBinaryServerSession* pSession = m_poolSessions.FetchObj();
	m_setSessions.insert(pSession);
	pSession->Init();
	return pSession;
}

bool BinaryServerSessionManager::Init()
{
	return m_poolSessions.Init(64, 64);
}

void BinaryServerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void BinaryServerSessionManager::Release(CBinaryServerSession * pSession)
{
	m_setSessions.erase(pSession);
	m_poolSessions.ReleaseObj(pSession);
}

