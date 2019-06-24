#include "GameManagerSession.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwGameManagerSessionBuffLen = 64 * 1024;
static char g_pGameManagerSessionBuf[g_dwGameManagerSessionBuffLen];

CGameManagerSession::CGameManagerSession()
	: m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(GameProto::GameManagerAckGameInfoResult::descriptor(), &CGameManagerSession::OnGameManagerAckGameInfoResult);
}

CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
	GameProto::GameNotifyGameManagerInfo oInfo;
	oInfo.set_dw_player_port(GameServer::Instance()->GetPlayerListenPort());
	oInfo.set_dw_server_port(GameServer::Instance()->GetServerListenPort());
	oInfo.set_dw_slave_server_port(GameServer::Instance()->GetSlaveServerListenPort());
	oInfo.set_qw_team_id(GameServer::Instance()->GetTeamId());
	oInfo.set_dw_team_server_id(GameServer::Instance()->GetTeamServerId());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
}

void CGameManagerSession::OnClose(void)
{
}

void CGameManagerSession::OnError(unsigned int dwErrorNo)
{
}

void CGameManagerSession::OnRecv(const char * pBuf, unsigned int dwLen)
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

void CGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

bool CGameManagerSession::OnGameManagerAckGameInfoResult(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::GameManagerAckGameInfoResult* pMsg = dynamic_cast<GameProto::GameManagerAckGameInfoResult*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	if (pMsg->dw_result() != 0)
	{
		exit(-1);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
CBinaryGameManagerSession::CBinaryGameManagerSession()
{
}

CBinaryGameManagerSession::~CBinaryGameManagerSession()
{
}

void CBinaryGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}
