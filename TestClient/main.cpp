//#include <WinSock2.h>
#include "SocketSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#include "lua_engine.h"
#include "fxtimer.h"

int main()
{
	LogThread::CreateInstance();
	LogThread::Instance()->Init();

	CLuaEngine::CreateInstance();

	CLuaEngine::Instance()->Reload();

	GetTimeHandler()->Init();
	GetTimeHandler()->Run();

	IFxNet* pNet = FxNetGetModule();

	UINT32 dwIP = inet_addr("127.0.0.1");

	FxSession* pSession = oSessionFactory.CreateSession();
	pNet->Connect(pSession, dwIP, 12000, true);

	//char szMsg[1024] = "";
	//int i = 0;
	while (true)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		if (pSession->IsConnected())
		{
			//sprintf(szMsg, "%s", "select * from role");
			//++i;
			//sprintf(szMsg, "%d", ++i);
			//pSession->Send(szMsg, 1024);
			//i %= 20;
			//if (i == 0)
			//{
			//	pSession->Close();
			//}
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
}
