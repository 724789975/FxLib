#include "GameScene.h"
#include "GameConfigBase.h"
#include "../json/json.h"
#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"
#include "fxredis.h"
#include "Player.h"
#include "PlayerSession.h"

class RedisSetPlayerTeamId : public IRedisQuery
{
public:
	RedisSetPlayerTeamId(UINT64 qwTeamId, UINT64 qwPlayerId) : m_qwTeamId(qwTeamId), m_qwPlayerId(qwPlayerId) {}
	~RedisSetPlayerTeamId() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[64] = { 0 };
		sprintf(szQuery, "SET %llu_%s %llu", m_qwPlayerId, RedisConstant::szPlayerTeamId, m_qwTeamId);
		poDBConnection->Query(szQuery);
	}
	virtual void OnResult(void) { }
	virtual void Release(void) { }

private:
	UINT64 m_qwTeamId;
	UINT64 m_qwPlayerId;
};

class RedisDelPlayerTeamId : public IRedisQuery
{
public:
	RedisDelPlayerTeamId(UINT64 qwPlayerId) : m_qwPlayerId(qwPlayerId) {}
	~RedisDelPlayerTeamId() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		char szQuery[64] = { 0 };
		sprintf(szQuery, "DEL %llu_%s", m_qwPlayerId, RedisConstant::szPlayerTeamId);
		poDBConnection->Query(szQuery);
	}
	virtual void OnResult(void) { }
	virtual void Release(void) { }

private:
	UINT64 m_qwPlayerId;
};

CGameSceneBase::CGameSceneBase()
{
}


CGameSceneBase::~CGameSceneBase()
{
}

bool CGameSceneBase::Init(unsigned int dwGameType, std::string szRoles, UINT64 qwTeamId)
{
	switch ((GameProto::EGameType)(dwGameType))
	{
		case GameProto::GT_Common:
		{
			if (CreateInstance(new CGameSceneCommon) == false)
			{
				return false;
			}
		}
		break;
		default:
			return false;
	}
	Json::Value jRoles;
	Json::Reader jReader;

	if (!jReader.parse(szRoles, jRoles))
	{
		return false;
	}
	if (!jRoles.isArray())
	{
		return false;
	}
	if (jRoles.size() > MAXCLIENTNUM)
	{
		Assert(0);
	}
	for (int i = 0; i < jRoles.size(); ++i)
	{
		Instance()->m_qwRoles[i] = jRoles[i].asInt64();
		if (Instance()->m_qwRoles[i])
		{
			RedisSetPlayerTeamId oRedisSetPlayerTeamId(qwTeamId, Instance()->m_qwRoles[i]);
			FxRedisGetModule()->QueryDirect(&oRedisSetPlayerTeamId);
		}
	}

	Instance()->m_dwGameType = dwGameType;
	Instance()->m_qwTeamId = qwTeamId;
	Instance()->m_dwGameStartTime = GetTimeHandler()->GetSecond();
	return Instance()->Init();
}

void CGameSceneBase::Run(double fTime)
{
	switch (GetSceneState())
	{
		case GameProto::ESS_None: break;
		case GameProto::ESS_Prepare: { Preparing(fTime); } break;
		case GameProto::ESS_GameReady: { GameReady(fTime); } break;
		case GameProto::ESS_Gaming: { Gaming(fTime); } break;
		case GameProto::ESS_Transact: { Transacting(fTime); } break;
		default: {Assert(0); } break;
	}
}

void CGameSceneBase::Preparing(double fTime)
{
	static UINT32 s_dwNotifyTime = m_dwGameStartTime + 5;

	INT32 dwLeftTime = m_dwGameStartTime + CGameConfigBase::Instance()->GetPrepareTime() - GetTimeHandler()->GetSecond();
	if (dwLeftTime < 0)
	{
		ChangeState(GameProto::ESS_GameReady);
		return;
	}
	GameProto::GameNotifyPlayerPrepareTime oNotify;
	oNotify.set_dw_left_time(dwLeftTime);
	if (GetTimeHandler()->GetSecond() >= s_dwNotifyTime)
	{
		NotifyPlayer(oNotify);

		//通知客户端频率 最后5秒 每秒一次 前面 5秒一次
		if (s_dwNotifyTime >= m_dwGameStartTime + CGameConfigBase::Instance()->GetPrepareTime() - 5)
		{
			s_dwNotifyTime += 1;
		}
		else
		{
			s_dwNotifyTime += 5;
		}
	}
}

void CGameSceneBase::GameReady(double fTime)
{
	static UINT32 s_dwNotifyTime = m_dwGameStartTime + CGameConfigBase::Instance()->GetPrepareTime() + 1;

	INT32 dwLeftTime = m_dwGameStartTime + CGameConfigBase::Instance()->GetPrepareTime() + CGameConfigBase::Instance()->GetGameReadyTime() - GetTimeHandler()->GetSecond();
	if (dwLeftTime < 0)
	{
		ChangeState(GameProto::ESS_Gaming);
		return;
	}
	GameProto::GameNotifyPlayerGameReadyTime oNotify;
	oNotify.set_dw_left_time(dwLeftTime);
	if (GetTimeHandler()->GetSecond() >= s_dwNotifyTime)
	{
		NotifyPlayer(oNotify);
		s_dwNotifyTime += 1;
	}
}

void CGameSceneBase::Gaming(double fTime)
{

}

void CGameSceneBase::Transacting(double fTime)
{

}

void CGameSceneBase::OnPrepare()
{
}

void CGameSceneBase::OnGameReady()
{
}

void CGameSceneBase::OnGameStart()
{
}

void CGameSceneBase::OnTransact()
{
}

void CGameSceneBase::GameEnd()
{
	LogExe(LogLv_Debug, "%s", "game end~~~~~");
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i])
		{
			RedisDelPlayerTeamId oRedisDelPlayerTeamId(m_qwRoles[i]);
			FxRedisGetModule()->QueryDirect(&oRedisDelPlayerTeamId);
		}
	}
}

void CGameSceneBase::NotifyPlayer(google::protobuf::Message& refMsg)
{
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		CPlayerBase* pPlayerBase = GetPlayer(m_qwRoles[i]);
		if (!pPlayerBase)
		{
			continue;
		}
		if (!pPlayerBase->GetPlayerSession())
		{
			continue;
		}
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(refMsg, pBuf, dwBufLen);
		pPlayerBase->GetPlayerSession()->Send(pBuf, dwBufLen);
	}
}

void CGameSceneBase::NotifyPlayerExcept(google::protobuf::Message& refMsg, UINT64 qwPlayerId)
{
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i] == qwPlayerId)
		{
			continue;
		}
		CPlayerBase* pPlayerBase = GetPlayer(m_qwRoles[i]);
		if (!pPlayerBase)
		{
			continue;
		}
		if (!pPlayerBase->GetPlayerSession())
		{
			continue;
		}
		char* pBuf = NULL;
		unsigned int dwBufLen = 0;
		ProtoUtility::MakeProtoSendBuffer(refMsg, pBuf, dwBufLen);
		pPlayerBase->GetPlayerSession()->Send(pBuf, dwBufLen);
	}
}

void CGameSceneBase::ChangeState(GameProto::EGameSceneState eGameSceneState)
{
	LogExe(LogLv_Info, "game change state : %d", (int)eGameSceneState);
	SetSceneState(eGameSceneState);

	GameProto::GameNotifyPlayerGameState oNotify;
	oNotify.set_state(eGameSceneState);
	NotifyPlayer(oNotify);

	switch (GetSceneState())
	{
		case GameProto::ESS_Prepare: OnPrepare(); break;
		case GameProto::ESS_GameReady: OnGameReady(); break;
		case GameProto::ESS_Gaming: OnGameStart(); break;
		case GameProto::ESS_Transact: OnTransact(); break;
		default:Assert(0); break;
	}
}

//////////////////////////////////////////////////////////////////////////
CGameSceneCommon::CGameSceneCommon()
{
	SetSceneState(GameProto::ESS_None);
}

CGameSceneCommon::~CGameSceneCommon()
{
}

bool CGameSceneCommon::Init()
{
	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		if (m_qwRoles[i] != 0)
		{
			m_mapPlayers[m_qwRoles[i]].SetPlayerSession(NULL);
			m_mapPlayers[m_qwRoles[i]].SetPlayerId(m_qwRoles[i]);
		}
	}
	ChangeState(GameProto::ESS_Prepare);
	return true;
}

CPlayerBase* CGameSceneCommon::GetPlayer(UINT64 qwPlayerId)
{
	std::map<UINT64, CCommonPlayer>::iterator it = m_mapPlayers.find(qwPlayerId);
	if (it == m_mapPlayers.end())
	{
		return NULL;
	}
	return &(it->second);
}

void CGameSceneCommon::OnGameStart()
{
	for (std::map<UINT64, CCommonPlayer>::iterator it = m_mapPlayers.begin();
		it != m_mapPlayers.end(); ++it)
	{
		it->second.Init();
	}
}

void CGameSceneCommon::Gaming(double fTime)
{
	for (std::map<UINT64, CCommonPlayer>::iterator it = m_mapPlayers.begin();
		it != m_mapPlayers.end(); ++it)
	{
		it->second.Update(fTime);
	}
}

GameProto::EGameSceneState CGameSceneCommon::GetSceneState()
{
	return m_oSceneInfo.mutable_scene_info()->state();
}

void CGameSceneCommon::FillProtoScene(GameProto::GameNotifyPlayerGameSceneInfo& refInfo)
{
	refInfo.set_dw_game_type(m_dwGameType);
	*(refInfo.mutable_common_scene_info()) = m_oSceneInfo;

	for (int i = 0; i < MAXCLIENTNUM; ++i)
	{
		GameProto::RoleData* pData = refInfo.add_players();
		if (m_qwRoles[i] != 0)
		{
			m_mapPlayers[m_qwRoles[i]].FillPlayerData(*pData);
		}
	}
}

void CGameSceneCommon::SetSceneState(GameProto::EGameSceneState eGameSceneState)
{
	m_oSceneInfo.mutable_scene_info()->set_state(eGameSceneState);
}

