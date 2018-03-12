#include "GameManagerSession.h"
#include "gamedefine.h"
#include "GameServer.h"
#include "msg_proto/web_game.pb.h"

const static unsigned int g_dwGameManagerSessionBuffLen = 64 * 1024;
static char g_pGameManagerSessionBuf[g_dwGameManagerSessionBuffLen];

CGameManagerSession::CGameManagerSession()
	: m_oProtoDispatch(*this)
{
	m_oProtoDispatch.RegistFunction(game_proto::GameManagerAckGameInfoResult::descriptor(), &CGameManagerSession::OnGameManagerAckGameInfoResult);
}

CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
	game_proto::GameNotifyGameManagerInfo oInfo;
	oInfo.set_dw_player_port(GameServer::Instance()->GetPlayerListenPort());
	oInfo.set_dw_server_port(GameServer::Instance()->GetServerListenPort());
	oInfo.set_dw_slave_server_port(GameServer::Instance()->GetSlaveServerListenPort());
	oInfo.set_qw_player_point(GameServer::Instance()->GetPlayerPoint());
	CNetStream oStream(ENetStreamType_Write, g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen);
	oStream.WriteString(oInfo.GetTypeName());

	std::string szInfo = oInfo.SerializeAsString();
	oStream.WriteData(szInfo.c_str(), szInfo.size());
	Send(g_pGameManagerSessionBuf, g_dwGameManagerSessionBuffLen - oStream.GetDataLength());
}

void CGameManagerSession::OnClose(void)
{
}

void CGameManagerSession::OnError(UINT32 dwErrorNo)
{
}

void CGameManagerSession::OnRecv(const char * pBuf, UINT32 dwLen)
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

void CGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

bool CGameManagerSession::OnGameManagerAckGameInfoResult(CGameManagerSession& refSession, google::protobuf::Message& refMsg)
{
	game_proto::GameManagerAckGameInfoResult* pMsg = dynamic_cast<game_proto::GameManagerAckGameInfoResult*>(&refMsg);
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
