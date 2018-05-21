#include "PlayerSession.h"
#include "netstream.h"
#include "msg_proto/web_game.pb.h"
#include "GameScene.h"
#include "Player.h"
#include "GameConfigBase.h"
//#include "gamedefine.h"

const static unsigned int g_dwPlayerSessionBuffLen = 64 * 1024;
static char g_pPlayerSessionBuf[g_dwPlayerSessionBuffLen];

CPlayerSession::CPlayerSession()
	: m_oProtoDispatch(*this)
	, m_qwPlayerId(0)
{
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestGameTest::descriptor(), &CPlayerSession::OnPlayerRequestGameTest);
	m_oProtoDispatch.RegistFunction(GameProto::PlayerRequestGameEnter::descriptor(), &CPlayerSession::OnPlayerRequestGameEnter);
}

CPlayerSession::~CPlayerSession()
{
}

void CPlayerSession::OnConnect(void)
{
}

void CPlayerSession::OnClose(void)
{
	CPlayerBase* pPlayer = CGameSceneBase::Instance()->GetPlayer(m_qwPlayerId);
	pPlayer->SetPlayerSession(NULL);
	m_qwPlayerId = 0;
}

void CPlayerSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CPlayerSession::OnRecv(const char* pBuf, UINT32 dwLen)
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

void CPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

bool CPlayerSession::OnPlayerRequestGameTest(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestGameTest* pMsg = dynamic_cast<GameProto::PlayerRequestGameTest*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	LogExe(LogLv_Debug, "recv : %s", pMsg->sz_test().c_str());

	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(*pMsg, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	return true;
}

bool CPlayerSession::OnPlayerRequestGameEnter(CPlayerSession& refSession, google::protobuf::Message& refMsg)
{
	GameProto::PlayerRequestGameEnter* pMsg = dynamic_cast<GameProto::PlayerRequestGameEnter*>(&refMsg);
	if (pMsg == NULL)
	{
		return false;
	}

	GameProto::GameAckPlayerEnter oResult;
	CPlayerBase* pPlayerBase = CGameSceneBase::Instance()->GetPlayer(pMsg->qw_player_id());
	if (!pPlayerBase)
	{
		LogExe(LogLv_Critical, "player : %llu not in game", pMsg->qw_player_id());
		oResult.set_dw_result(GameProto::EC_CannotFindPlayer);
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
		Send(pBuf, dwBufLen);
		return true;
	}
	pPlayerBase->SetPlayerSession(this);
	m_qwPlayerId = pMsg->qw_player_id();
	char* pBuf = NULL;
	unsigned int dwBufLen = 0;
	ProtoUtility::MakeProtoSendBuffer(oResult, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	GameProto::GameNotifyPlayerGameConfig oNotifyConfig;
	CGameConfigBase::Instance()->FillProtoConfig(oNotifyConfig);
	ProtoUtility::MakeProtoSendBuffer(oNotifyConfig, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);

	GameProto::GameNotifyPlayerGameSceneInfo oSceneInfo;
	CGameSceneBase::Instance()->FillProtoConfig(oSceneInfo);
	ProtoUtility::MakeProtoSendBuffer(oSceneInfo, pBuf, dwBufLen);
	Send(pBuf, dwBufLen);
	return true;
}

//////////////////////////////////////////////////////////////////////////
CWebSocketPlayerSession::CWebSocketPlayerSession()
{
}

CWebSocketPlayerSession::~CWebSocketPlayerSession()
{
}

void CWebSocketPlayerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

//////////////////////////////////////////////////////////////////////////
WebSocketPlayerSessionManager::WebSocketPlayerSessionManager()
{
}

WebSocketPlayerSessionManager::~WebSocketPlayerSessionManager()
{
}

FxSession * WebSocketPlayerSessionManager::CreateSession()
{
	FxSession* pSession = NULL;
	m_oLock.Lock();
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_oWebSocketPlayerSessions[i].GetConnection() == NULL)
		{
			m_oWebSocketPlayerSessions[i].Init((FxConnection*)0xFFFFFFFF);
			pSession = &m_oWebSocketPlayerSessions[i];
			break;
		}
	}
	m_oLock.UnLock();
	return pSession;
}

void WebSocketPlayerSessionManager::Release(FxSession * pSession)
{
	Assert(0);
}

void WebSocketPlayerSessionManager::Release(CWebSocketPlayerSession* pSession)
{
	m_oLock.Lock();
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (&m_oWebSocketPlayerSessions[i] == pSession)
		{
			m_oWebSocketPlayerSessions[i].Init(NULL);
			break;
		}
	}
	m_oLock.UnLock();
}
