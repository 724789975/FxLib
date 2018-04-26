#include "ServerSession.h"
#include "netstream.h"
#include "gamedefine.h"
#include "PlayerSession.h"
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
	LogExe(LogLv_Debug, "protocol name : %s", szProtocolName.c_str());
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

	Init(NULL);
}

bool CServerSession::OnGameNotifyGameManagerInfo(CServerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::GameNotifyGameManagerInfo* pMsg = dynamic_cast<GameProto::GameNotifyGameManagerInfo*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameProto::GameManagerAckGameInfoResult oResult;
	std::map<unsigned int, CBinaryTeamSession *>& refSSessions = GameServer::Instance()->GetTeamSessionManager().GetTeamSessions();
	std::map<unsigned int, CBinaryTeamSession *>::iterator it = refSSessions.find(pMsg->dw_team_server_id());
	if (it == refSSessions.end())
	{
		oResult.set_dw_result(GameProto::EC_NoTeamServer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		LogExe(LogLv_Critical, "find team server error, team server id : %d", pMsg->dw_team_server_id());
		return true;
	}
	oResult.set_dw_result(0);

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	GameProto::GameManagerAckTeamGameStart oGameManagerAckTeamGameStart;
	oGameManagerAckTeamGameStart.set_dw_result(0);
	oGameManagerAckTeamGameStart.set_dw_player_port(pMsg->dw_player_port());
	oGameManagerAckTeamGameStart.set_dw_server_port(pMsg->dw_server_port());
	oGameManagerAckTeamGameStart.set_dw_slave_server_port(pMsg->dw_slave_server_port());
	oGameManagerAckTeamGameStart.set_qw_team_id(pMsg->qw_team_id());
	oGameManagerAckTeamGameStart.set_sz_listen_ip(GameServer::Instance()->GetServerIp());
	char* pBufStart = NULL;
	unsigned int dwBufStartLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oGameManagerAckTeamGameStart, pBufStart, dwBufStartLen);
	it->second->Send(pBufStart, dwBufStartLen);
	return true;
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

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
CBinaryServerSession * BinaryServerSessionManager::CreateSession()
{
	CBinaryServerSession* pSession = m_poolSessions.FetchObj();
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
	m_poolSessions.ReleaseObj(pSession);
}

