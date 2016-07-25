#include "SocketSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua_engine.h"
#include "fxtimer.h"

#include <signal.h>

#define CLIENTCOUNT 1

char* g_strIp = "127.0.0.1";
unsigned int g_dwPort = 12000;
bool g_bRun = true;

void EndFun(int n)
{
	if (n == SIGINT || n == SIGTERM)
	{
		g_bRun = false;
	}
	else
	{
		printf("unknown signal : %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", n);
	}
}

int main(int argc, char **argv)
{
	//--------------------order can't change begin-------------------------//
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
	if (!CLuaEngine::Instance()->Reload())
	{
		return 0;
	}
	// must defined before goto
	IFxNet* pNet = NULL;
	UINT32 dwIP = 0;
	FxSession* oSessions[CLIENTCOUNT] = { 0 };
	char szMsg[1024] = "";
	int j = 0;

	if (!CLuaEngine::Instance()->CommandLineFunction(argv, argc))
	{
		g_bRun = false;
		goto STOP;
	}
	if (!GetTimeHandler()->Init())
	{
		g_bRun = false;
		goto STOP;
	}
	GetTimeHandler()->Run();
	if (!LogThread::Instance()->Init())
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
	//--------------------order can't change end-------------------------//

	dwIP = inet_addr(g_strIp);

	for (int i = 0; i < CLIENTCOUNT; ++i)
	{
		oSessions[i] = oSessionFactory.CreateSession();
		pNet->UdpConnect(oSessions[i], dwIP, g_dwPort, true);
	}

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		for (int i = 0; i < CLIENTCOUNT; ++i)
		{
			if (oSessions[i]->IsConnected())
			{
				sprintf(szMsg, "%d", j);
				if (!oSessions[i]->Send(szMsg, 1024))
				{
					//pSession->Close();
				}
				else
				{
					LogExe(LogLv_Debug, "send : %s", szMsg);
					//++i;
				}
				//FxSleep(1);
			}
			else
			{
				if (!oSessions[i]->IsConnecting())
				{
					oSessions[i]->Reconnect();
				}
			}
		}
		FxSleep(1);
		++j;
	}
	for (int i = 0; i < CLIENTCOUNT; ++i)
	{
		oSessions[i]->Close();
	}
	FxSleep(10);
	pNet->Run(0xffffffff);
	FxSleep(10);
	pNet->Release();
STOP:
	LogThread::Instance()->Stop();
}
