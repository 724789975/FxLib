#include "Timers.h"
#include "GameServer.h"
#include "GameScene.h"

extern bool g_bRun;

bool GameEnd::OnTimer(double fSecond)
{
	LogExe(LogLv_Debug, "%s", "time end~~~~~");
	CGameSceneBase::Instance()->GameEnd();
	GameServer::Instance()->GameEnd();
	g_bRun = false;
	return true;
}
