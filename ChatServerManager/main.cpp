#include "SocketSession.h"
#include "lua_engine.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"
#include "ChatServerManager.h"
#include "gflags/gflags.h"

#include <signal.h>

bool g_bRun = true;

DEFINE_uint32(chat_server_port, 13000, "chat server listen port");

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
	gflags::SetUsageMessage("ChatServerManager");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);
	if (!LogThread::CreateInstance())
	{
		return 0;
	}

	if (!CLuaEngine::CreateInstance())
	{
		return 0;
	}
	std::vector<ToluaFunctionOpen*> vecFunctions;
	int tolua_LuaMeta_open(lua_State*);
	vecFunctions.push_back(tolua_LuaMeta_open);
	if (!CLuaEngine::Instance()->Init(vecFunctions))
	{
		return 0;
	}
	if (!CLuaEngine::Instance()->Reload("ChatServerManagerScripts"))
	{
		return 0;
	}
	// must define before goto
	IFxNet* pNet = NULL;
	IFxListenSocket* pChatServerListenSocket = NULL;

	//if (!CLuaEngine::Instance()->CommandLineFunction(argv, argc))
	//{
	//	return 0;
	//}
	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	if (!LogThread::Instance()->Init())
	{
		g_bRun = false;
		goto STOP;
	}

	if (!ChatServerManager::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}
	ChatServerManager::Instance()->Init();
	pNet = FxNetGetModule();
	if (!pNet)
	{
		g_bRun = false;
		goto STOP;
	}
	//----------------------order can't change end-----------------------//

	pChatServerListenSocket = pNet->Listen(&ChatServerManager::Instance()->GetChatSessionManager(), SLT_CommonTcp, 0, FLAGS_chat_server_port);
	if(pChatServerListenSocket == NULL)
	{
		g_bRun = false;
		goto STOP;
	}
	while (g_bRun)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		//LogFun(LT_Screen, LogLv_Info, "%s", PrintTrace());
		FxSleep(1);
	}
	pChatServerListenSocket->StopListen();
	pChatServerListenSocket->Close();

	ChatServerManager::Instance()->GetChatSessionManager().CloseSessions();

	FxSleep(10);
	pNet->Release();
STOP:
	LogThread::Instance()->Stop();
}
