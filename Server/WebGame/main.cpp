#include "SocketSession.h"
#include "fxtimer.h"
#include "fxmeta.h"
#include "GameServer.h"
#include "fxredis.h"
#include "GameConfigBase.h"
#include "GameScene.h"
#include "Timers.h"
#include <signal.h>
#include "gflags/gflags.h"

bool g_bRun = true;

DEFINE_string(game_manager_ip, "127.0.0.1", "game manager ip");
DEFINE_uint32(game_manager_port, 30001, "game manager port");
DEFINE_uint64(team_id, 0, "team id");
DEFINE_uint32(game_time, 180, "game_time");
DEFINE_string(roles, "[]", "roles");
DEFINE_uint32(team_server_id, 0, "team_server_id");
DEFINE_uint32(game_type, 1, "game_type");
DEFINE_string(redis_ip, "127.0.0.1", "redis ip");
DEFINE_string(redis_pw, "1", "redis password");
DEFINE_uint32(redis_port, 16379, "redis port");

void EndFun(int n)
{
	if (n == SIGINT || n == SIGTERM)
	{
		g_bRun = false;
	}
	else
	{
		printf("unknown signal : %d !!!!!!!!!!!!!!!!!!!!!!!!!!!\n", n);
	}
}

int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("WebGame");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	IFxNet* pNet = FxNetGetModule();
	GameEnd oGameEnd;
	GameStart oGameStart;
	// must define before goto
	if (!GetTimeHandler()->Init())
	{
		g_bRun = false;
		goto STOP;
	}
	GetTimeHandler()->Run();

	if (!pNet)
	{
		g_bRun = false;
		goto STOP;
	}

	if (!GameServer::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}

	//----------------------order can't change end-----------------------//

	GetTimeHandler()->AddTimer(FLAGS_game_time, &(oGameEnd.MakeEvent(&oGameEnd, &GameEnd::OnTimer)));

	if (!FxRedisGetModule()->Open(FLAGS_redis_ip, FLAGS_redis_port, FLAGS_redis_pw, 0))
	{
		LogExe(LogLv_Error, "%s", "redis connected failed~~~~");
		goto STOP;
	}

	if (!CGameConfigBase::Init(FLAGS_game_type))
	{
		LogExe(LogLv_Error, "%s", "game config init failed~~~~");
		goto STOP;
	}
	if (!CGameSceneBase::Init(FLAGS_game_type, FLAGS_roles, FLAGS_team_id))
	{
		LogExe(LogLv_Error, "%s", "game scene init failed~~~~");
		goto STOP;
	}

	if (!GameServer::Instance()->Init(FLAGS_game_manager_ip, FLAGS_game_manager_port, FLAGS_team_id, FLAGS_team_server_id))
	{
		g_bRun = false;
		goto STOP;
	}

	//GetTimeHandler()->AddDelayTimer(CGameConfigBase::Instance()->GetPrepareTime(), &oGameStart);

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		CGameSceneBase::Instance()->Run(GetTimeHandler()->GetDeltaTime());
		pNet->Run(0xffffffff);
		FxSleep(1);
	}
	GameServer::Instance()->Stop();
	CGameSceneBase::Instance()->GameEnd();
	GameServer::Instance()->GameEnd();
	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");
}
