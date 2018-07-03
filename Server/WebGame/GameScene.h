#ifndef __GameScene_H__
#define __GameScene_H__

#include "fxmeta.h"
#include "gamedefine.h"
#include <map>
#include "Player.h"
#include "msg_proto/web_data.pb.h"

class CGameSceneBase : public TSingleton<CGameSceneBase>
{
public:
	CGameSceneBase();
	virtual ~CGameSceneBase();

	static bool Init(unsigned int dwGameType, std::string szRoles, UINT64 qwTeamId);
	virtual bool Init() = 0;

	virtual void Run(double fTime);
	virtual void Preparing(double fTime);
	virtual void GameReady(double fTime);
	virtual void Gaming(double fTime);
	virtual void Transacting(double fTime);
	virtual void OnPrepare();
	virtual void OnGameReady();
	virtual void OnGameStart();
	virtual void OnTransact();

	virtual CPlayerBase* GetPlayer(UINT64 qwPlayerId) = 0;
	virtual void GameEnd();

	void NotifyPlayer(google::protobuf::Message& refMsg);
	void NotifyPlayerExcept(google::protobuf::Message& refMsg, UINT64 qwPlayerId);

	void ChangeState(GameProto::EGameSceneState eGameSceneState);

	virtual void FillProtoScene(GameProto::GameNotifyPlayerGameSceneInfo& refInfo) = 0;

	virtual GameProto::EGameSceneState GetSceneState() = 0;

	UINT64* GetPlayers() { return m_qwRoles; }

protected:

	virtual void SetSceneState(GameProto::EGameSceneState eGameSceneState) = 0;
	GameProto::EGameSceneState m_eGameSceneState;

	UINT32 m_dwGameType;

	UINT64 m_qwRoles[MAXCLIENTNUM];
	UINT64 m_qwTeamId;

	UINT32 m_dwGameStartTime;
};

class CGameSceneCommon : public CGameSceneBase
{
public:
	CGameSceneCommon();
	virtual ~CGameSceneCommon();

	virtual bool Init();

	virtual CPlayerBase * GetPlayer(UINT64 qwPlayerId);

	virtual void OnGameStart();
	virtual void Gaming(double fTime);

	virtual GameProto::EGameSceneState GetSceneState();
	virtual void FillProtoScene(GameProto::GameNotifyPlayerGameSceneInfo& refInfo);
protected:
	virtual void SetSceneState(GameProto::EGameSceneState eGameSceneState);
private:
	std::map<UINT64, CCommonPlayer> m_mapPlayers;

	GameProto::GameCommonSceneInfo m_oSceneInfo;
};


#endif // __GameScene_H__
