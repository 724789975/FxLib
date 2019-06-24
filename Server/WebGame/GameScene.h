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

	static bool Init(unsigned int dwGameType, std::string szRoles, unsigned long long qwTeamId);
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

	virtual CPlayerBase* GetPlayer(unsigned long long qwPlayerId) = 0;
	virtual void GameEnd();

	void NotifyPlayer(google::protobuf::Message& refMsg);
	void NotifyPlayerExcept(google::protobuf::Message& refMsg, unsigned long long qwPlayerId);

	void ChangeState(GameProto::EGameSceneState eGameSceneState);

	virtual void FillProtoScene(GameProto::GameNotifyPlayerGameSceneInfo& refInfo) = 0;

	virtual GameProto::EGameSceneState GetSceneState() = 0;

	unsigned long long* GetPlayers() { return m_qwRoles; }

protected:

	virtual void SetSceneState(GameProto::EGameSceneState eGameSceneState) = 0;
	GameProto::EGameSceneState m_eGameSceneState;

	unsigned int m_dwGameType;

	unsigned long long m_qwRoles[MAXCLIENTNUM];
	unsigned long long m_qwTeamId;

	unsigned int m_dwGameStartTime;
};

class CGameSceneCommon : public CGameSceneBase
{
public:
	CGameSceneCommon();
	virtual ~CGameSceneCommon();

	virtual bool Init();

	virtual CPlayerBase * GetPlayer(unsigned long long qwPlayerId);

	virtual void OnGameStart();
	virtual void Gaming(double fTime);

	virtual GameProto::EGameSceneState GetSceneState();
	virtual void FillProtoScene(GameProto::GameNotifyPlayerGameSceneInfo& refInfo);
protected:
	virtual void SetSceneState(GameProto::EGameSceneState eGameSceneState);
private:
	std::map<unsigned long long, CCommonPlayer> m_mapPlayers;

	GameProto::GameCommonSceneInfo m_oSceneInfo;
};


#endif // __GameScene_H__
