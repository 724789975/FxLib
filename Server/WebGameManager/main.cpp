#include "SocketSession.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"
#include "PlayerSession.h"
#include "ServerSession.h"
#include "GameServer.h"

#include <signal.h>
#include "gflags/gflags.h"

unsigned int g_dwPort = 20000;
bool g_bRun = true;

DEFINE_uint32(server_id, 30001, "server id");
DEFINE_string(server_ip, "127.0.0.1", "server ip");
DEFINE_uint32(server_port, 31001, "server port");
DEFINE_uint32(player_port, 31002, "player port");
DEFINE_string(center_ip, "127.0.0.1", "center ip");
DEFINE_uint32(center_port, 40000, "center port");
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
		printf("unknown signal : %d !!!!!!!!!!!!!!!!!!!!!!!\n", n);
	}
}

int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("WebGameManager");
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
	//----------------------order can't change end-----------------------//

	if (!GameServer::Instance()->Init(FLAGS_server_id, FLAGS_server_ip, FLAGS_center_ip, FLAGS_center_port, FLAGS_server_port, FLAGS_player_port))
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
	printf("error!!!!!!!!\n");false;}
