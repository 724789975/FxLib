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
	gflags::SetUsageMessage("TestServer");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	// must define before goto
	IFxNet* pNet = NULL;
	IFxListenSocket* pServerListenSocket = NULL;
	IFxListenSocket* pPlayerListenSocket = NULL;

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

	UINT16 wPort = FLAGS_server_port;
	pServerListenSocket = pNet->Listen(&GameServer::Instance()->GetBinaryServerSessionManager(), SLT_CommonTcp, 0, wPort);
	if (pServerListenSocket == NULL)
	{
		g_bRun = false;
		goto STOP;
	}

	wPort = FLAGS_player_port;
	pPlayerListenSocket = pNet->Listen(&GameServer::Instance()->GetWebSocketPlayerSessionManager(), SLT_WebSocket, 0, wPort);
	if(pPlayerListenSocket == NULL)
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
	pServerListenSocket->StopListen();
	pServerListenSocket->Close();
	pPlayerListenSocket->StopListen();
	pPlayerListenSocket->Close();

	GameServer::Instance()->Stop();

	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");
}
