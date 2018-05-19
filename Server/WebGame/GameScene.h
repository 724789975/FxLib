#ifndef __GameScene_H__
#define __GameScene_H__

#include "fxmeta.h"
#include "gamedefine.h"
#include <map>
#include "Player.h"

class CGameSceneBase : public TSingleton<CGameSceneBase>
{
public:
	CGameSceneBase();
	virtual ~CGameSceneBase();

	static bool Init(unsigned int dwGameType, std::string szRoles, UINT64 qwTeamId);
	virtual bool Init() = 0;

	virtual CPlayerBase* GetPlayer(UINT64 qwPlayerId) = 0;
	virtual void GameEnd();

protected:
	UINT64 m_qwRoles[MAXCLIENTNUM];
	UINT64 m_qwTeamId;
};

class CGameSceneCommon : public CGameSceneBase
{
public:
	CGameSceneCommon();
	virtual ~CGameSceneCommon();

	virtual bool Init();

	virtual CPlayerBase * GetPlayer(UINT64 qwPlayerId);
private:
	std::map<UINT64, CCommonPlayer> m_mapPlayers;
};


#endif // __GameScene_H__
