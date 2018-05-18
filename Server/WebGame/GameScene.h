#ifndef __GameScene_H__
#define __GameScene_H__

#include "fxmeta.h"
#include "gamedefine.h"

class CGameSceneBase : public TSingleton<CGameSceneBase>
{
public:
	CGameSceneBase();
	virtual ~CGameSceneBase();

	static bool Init(unsigned int dwGameType, std::string szRoles);
	virtual bool Init() = 0;

protected:
	UINT64 m_qwRoles[MAXCLIENTNUM];
};

class CGameSceneCommon : public CGameSceneBase
{
public:
	CGameSceneCommon();
	virtual ~CGameSceneCommon();

	virtual bool Init();
private:

};


#endif // __GameScene_H__
