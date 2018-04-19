#include "SocketSession.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"
#include "PlayerSession.h"
#include "ServerSession.h"
#include "GameServer.h"
#include "fxredis.h"

#include <signal.h>
#include "gflags/gflags.h"

bool g_bRun = true;

DEFINE_uint32(server_id, 10001, "server id");
DEFINE_uint32(player_port, 11001, "player port");
DEFINE_string(center_ip, "127.0.0.1", "center ip");
DEFINE_uint32(center_port, 40000, "center port");
DEFINE_uint32(login_port, 11002, "login port");
DEFINE_uint32(team_port, 11003, "team port");
DEFINE_uint32(game_manager_port, 11004, "game manager port");

void EndFun(int n)
{
	if (n == SIGINT || n == SIGTERM)
	{
		g_bRun = false;
	}
	else
	{
		printf("unknown signal : %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", n);
	}
}

int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("WebGameLogin");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	// must define before goto
	IFxNet* pNet = NULL;

	if (!GetTimeHandler()->Init())
	{
		g_bRun = false;
		goto STOP;
	}
	GetTimeHandler()->Run();

	if (!GameServer::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}

	pNet = FxNetGetModule();
	if (!pNet)
	{
		g_bRun = false;
		goto STOP;
	}
	if (!FxRedisGetModule()->Open("127.0.0.1", 16379, "1", 0))
	{
		LogExe(LogLv_Info, "%s", "redis connected failed~~~~");
		goto STOP;
	}
	//----------------------order can't change end-----------------------//

	if (!GameServer::Instance()->Init(FLAGS_server_id,
		"0.0.0.0", FLAGS_player_port, FLAGS_center_ip, FLAGS_center_port,
		FLAGS_login_port, FLAGS_team_port, FLAGS_game_manager_port))
	{
		g_bRun = false;
		goto STOP;
	}
	while (g_bRun)
	{
		GetTimeHandler()->Run();
		FxRedisGetModule()->Run();
		pNet->Run(0xffffffff);
		FxSleep(1);
	}

	GameServer::Instance()->Stop();

	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");false;}
