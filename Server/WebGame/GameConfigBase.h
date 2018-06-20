#ifndef __CGameConfigBase_H__
#define __CGameConfigBase_H__

#include "singleton.h"

#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"

class CGameConfigBase : public TSingleton<CGameConfigBase>
{
public:
	CGameConfigBase();
	virtual ~CGameConfigBase();

	static bool Init(unsigned int dwGameType);
	virtual bool Init() = 0;

	virtual void SetGameType(unsigned int dwGameType) = 0;
	virtual unsigned int GetGameType() = 0;
	virtual unsigned int GetPrepareTime() = 0;
	virtual unsigned int GetGameReadyTime() = 0;
	virtual unsigned int GetSuspendTime() = 0;

	virtual void FillProtoConfig(GameProto::GameNotifyPlayerGameConfig& refConfig) = 0;
};

class CGameCommonConfig : public CGameConfigBase
{
public:
	virtual bool Init();

	virtual void SetGameType(unsigned int dwGameType);
	virtual unsigned int GetGameType();
	virtual unsigned int GetPrepareTime();
	virtual unsigned int GetGameReadyTime();
	virtual unsigned int GetSuspendTime();

	virtual void FillProtoConfig(GameProto::GameNotifyPlayerGameConfig& refConfig);
protected:
private:

	GameProto::GameConfigCommon m_oConfig;
};

#endif	//__CGameConfigBase_H__
