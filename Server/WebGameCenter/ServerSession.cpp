#include "ServerSession.h"
#include "netstream.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"
#include "gamedefine.h"

const static unsigned int g_dwServerSessionBuffLen = 64 * 1024;
static char g_pServerSessionBuf[g_dwServerSessionBuffLen];

CServerSession::CServerSession()
	:m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::ServerInfo::descriptor(), &CServerSession::OnServerInfo);
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

	LogExe(LogLv_Debug, "server : %d connected, listen ip : %s login_port : %d, team_port : %d, game_manager_port : %d",
		m_dwServerId, GetRemoteIPStr(), m_dwLoginPort, m_dwTeamPort, m_dwGameServerManagerPort);

	enum GameProto::EServerType eServerType = (GameProto::EServerType)(m_dwServerId / 10000);
	switch (eServerType)
	{
	case GameProto::ST_Login:
	{
		// 如果是login连上了 那么 应该向其他的所有服务器发消息 让其他服务器连接这个login 包含其他的login
		std::set<CBinaryServerSession*>& refSessions = GameServer::Instance()->GetBinaryServerSessionManager().GetSessions();
		for (std::set<CBinaryServerSession*>::iterator it = refSessions.begin(); it != refSessions.end(); ++it)
		{
			if ((*it)->m_dwServerId == 0)
			{
				continue;
			}
			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(*pMsg, pBuf, dwBufLen);
			(*it)->Send(pBuf, dwBufLen);
		}
	}
		break;
	case GameProto::ST_Team:
	{
		// 如果是组队服务器 那么应该向这个服务器发送所有login的信息 让他去连login
		std::set<CBinaryServerSession*>& refSessions = GameServer::Instance()->GetBinaryServerSessionManager().GetSessions();
		for (std::set<CBinaryServerSession*>::iterator it = refSessions.begin(); it != refSessions.end(); ++it)
		{
			if ((*it)->m_dwServerId == 0)
			{
				continue;
			}
			GameProto::EServerType eCTeamType = (GameProto::EServerType)((*it)->m_dwServerId / 10000);
			switch (eCTeamType)
			{
			case GameProto::ST_Login:
			{
				GameProto::ServerInfo oInfo;
				oInfo.set_dw_server_id((*it)->m_dwServerId);
				oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
				oInfo.set_dw_login_port((*it)->m_dwLoginPort);
				oInfo.set_dw_team_port((*it)->m_dwTeamPort);
				oInfo.set_dw_game_server_manager_port((*it)->m_dwGameServerManagerPort);

				char* pBuf = NULL;
				unsigned int dwBufLen = 0;
				ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
				Send(pBuf, dwBufLen);
			}
				break;
			case GameProto::ST_GameManager:
			{
				char* pBuf = NULL;
				unsigned int dwBufLen = 0;
				ProtoUtility::MakeProtoSendBuffer(*pMsg, pBuf, dwBufLen);
				(*it)->Send(pBuf, dwBufLen);
			}
				break;
			default:
			{ continue; }
				break;
			}
		}
	}
		break;
	case GameProto::ST_GameManager:
	{
		// 如果是游戏管理服 那么应该向这个服务器发送所有login 和 team的信息 让他去连login team
		std::set<CBinaryServerSession*>& refSessions = GameServer::Instance()->GetBinaryServerSessionManager().GetSessions();
		for (std::set<CBinaryServerSession*>::iterator it = refSessions.begin(); it != refSessions.end(); ++it)
		{
			if ((*it)->m_dwServerId == 0)
			{
				continue;
			}
			if (((*it)->m_dwServerId / 10000 != GameProto::ST_Login) && ((*it)->m_dwServerId / 10000 != GameProto::ST_Team))
			{
				continue;
			}
			GameProto::ServerInfo oInfo;
			oInfo.set_dw_server_id((*it)->m_dwServerId);
			oInfo.set_sz_listen_ip((*it)->GetRemoteIPStr());
			oInfo.set_dw_login_port((*it)->m_dwLoginPort);
			oInfo.set_dw_team_port((*it)->m_dwTeamPort);
			oInfo.set_dw_game_server_manager_port((*it)->m_dwGameServerManagerPort);

			char* pBuf = NULL;
			unsigned int dwBufLen = 0;
			ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
			Send(pBuf, dwBufLen);
		}
	}
		break;
	default:
	{
		Assert(0);
	}
		break;
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

	//FxSession::Init(NULL);
	GameServer::Instance()->GetBinaryServerSessionManager().Release(this);
}

//////////////////////////////////////////////////////////////////////////
CBinaryServerSession * BinaryServerSessionManager::CreateSession()
{
	CBinaryServerSession* pSession = m_poolSessions.FetchObj();
	pSession->Init();
	m_setSessions.insert(pSession);
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

