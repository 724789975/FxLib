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
	oResult.set_dw_result(1);
	CPlayerSession* pPlayer = (CPlayerSession*)pMsg->qw_player_point();
	if (GameServer::Instance()->DelRequestPlayer(pPlayer))
	{
		oResult.set_dw_result(0);
		pPlayer->OnGameInfo(*pMsg);
	}
	CNetStream oWriteStream(ENetStreamType_Write, g_pServerSessionBuf, g_dwServerSessionBuffLen);
	oWriteStream.WriteString(oResult.GetTypeName());
	std::string szResult;
	oResult.SerializeToString(&szResult);
	oWriteStream.WriteData(szResult.c_str(), szResult.size());

	Send(g_pServerSessionBuf, g_dwServerSessionBuffLen - oWriteStream.GetDataLength());
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
FxSession * BinaryServerSessionManager::CreateSession()
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

