#include "GameScene.h"
#include "../json/json.h"
#include "msg_proto/web_data.pb.h"
#include "msg_proto/web_game.pb.h"


CGameSceneBase::CGameSceneBase()
{
}


CGameSceneBase::~CGameSceneBase()
{
}

bool CGameSceneBase::Init(unsigned int dwGameType, std::string szRoles)
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
	for (int i = 0; i < jRoles.size(); ++i)
	{
		Instance()->m_qwRoles[i] = jRoles[i].asInt64();
	}

	return Instance()->Init();
}

//////////////////////////////////////////////////////////////////////////
CGameSceneCommon::CGameSceneCommon()
{

}

CGameSceneCommon::~CGameSceneCommon()
{

}

bool CGameSceneCommon::Init()
{
	return true;
}
