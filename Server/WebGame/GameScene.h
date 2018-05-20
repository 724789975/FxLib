#ifndef __GameScene_H__
#define __GameScene_H__

#include "fxmeta.h"
#include "gamedefine.h"
#include <map>
#include "Player.h"

enum EGameSceneState
{
	ESS_None = 0,
	ESS_Prepare = 1,	//准备中
	ESS_GameReady = 10,	//敌军还有30秒到达战场
	ESS_Gaming = 20,	//游戏中
	ESS_Transact = 30,	//结算中
};

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

	void ChangeState(EGameSceneState eGameSceneState);

protected:

	EGameSceneState m_eGameSceneState;

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
	virtual void OnGameStart();

	virtual CPlayerBase * GetPlayer(UINT64 qwPlayerId);
private:
	std::map<UINT64, CCommonPlayer> m_mapPlayers;
};


#endif // __GameScene_H__
