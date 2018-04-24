#include "SocketSession.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"
#include "GameServer.h"

#include <signal.h>
#include "gflags/gflags.h"

bool g_bRun = true;

DEFINE_string(game_manager_ip, "127.0.0.1", "game manager ip");
DEFINE_uint32(game_manager_port, 30001, "game manager port");
DEFINE_uint64(team_id, 0, "player point");
DEFINE_uint32(game_time, 60, "game_time");
DEFINE_string(roles, "[]", "roles");
DEFINE_uint32(team_server_id, 0, "team_server_id");

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
	gflags::SetUsageMessage("WebGame");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	IFxNet* pNet = FxNetGetModule();
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

	GetTimeHandler()->AddDelayTimer(FLAGS_game_time, GameServer::Instance());
	if (!GameServer::Instance()->Init(inet_addr(FLAGS_game_manager_ip.c_str()), FLAGS_game_manager_port, FLAGS_team_id, FLAGS_team_server_id))
	{
		g_bRun = false;
		goto STOP;
	}

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		FxSleep(1);
	}
	GameServer::Instance()->Stop();
	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");
}
