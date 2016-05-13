#include "SocketSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua_engine.h"
#include "fxtimer.h"

#include <signal.h>

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
	LogThread::CreateInstance();

	CLuaEngine::CreateInstance();
	CLuaEngine::Instance()->Reload();

	GetTimeHandler()->Init();
	GetTimeHandler()->Run();
	LogThread::Instance()->Init();
	//--------------------order can't change end-------------------------//

	IFxNet* pNet = FxNetGetModule();

	UINT32 dwIP = inet_addr("127.0.0.1");

	FxSession* pSession = oSessionFactory.CreateSession();
	pNet->Connect(pSession, dwIP, 12000, true);

	char szMsg[1024] = "";
	int i = 0;
	while (bRun)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		if (pSession->IsConnected())
		{
			sprintf(szMsg, "%d", i);
			if (!pSession->Send(szMsg, 1024))
			{
				//pSession->Close();
			}
			else
			{
				LogFun(LT_Screen | LT_File, LogLv_Debug, "send : %s", szMsg);
				++i;
			}
			FxSleep(1);
		}
		else
		{
			if (!pSession->IsConnecting())
			{
				pSession->Reconnect();
			}
		}
	}
	pSession->Close();
	FxSleep(10);
	pNet->Run(0xffffffff);
	FxSleep(10);
	pNet->Release();
	LogThread::Instance()->Stop();
}
