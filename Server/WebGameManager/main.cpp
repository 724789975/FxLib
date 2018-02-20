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

DEFINE_uint32(server_port, 30001, "server port");
DEFINE_uint32(player_port, 30002, "player port");

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

	if (!GameServer::Instance()->Init("0.0.0.0", FLAGS_server_port, "0.0.0.0", FLAGS_player_port))
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
