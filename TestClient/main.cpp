#include "SocketSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua_engine.h"
#include "fxtimer.h"

#include <signal.h>

#define CLIENTCOUNT 256

bool bRun = true;

void EndFun(int n)
{
	if (n == SIGINT || n == SIGTERM)
	{
		bRun = false;
	}
	else
	{
		printf("unknown signal : %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", n);
	}
}

int main()
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
	if (!CLuaEngine::Instance()->Reload())
	{
		return 0;
	}

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	if (!LogThread::Instance()->Init())
	{
		return 0;
	}

	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return 0;
	}
	//--------------------order can't change end-------------------------//

	UINT32 dwIP = inet_addr("127.0.0.1");

	FxSession* oSessions[CLIENTCOUNT] = { 0 };

	for (int i = 0; i < CLIENTCOUNT; ++i)
	{
		oSessions[i] = oSessionFactory.CreateSession();
		pNet->Connect(oSessions[i], dwIP, 12000, true);
	}

	//FxSession* pSession = oSessionFactory.CreateSession();
	//pNet->Connect(pSession, dwIP, 12000, true);

	char szMsg[1024] = "";
	int j = 0;
	while (bRun)
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
	LogThread::Instance()->Stop();
}
