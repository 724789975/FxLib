#include "SocketSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua_engine.h"
#include "fxtimer.h"
#include "ifnet.h"

#include <signal.h>

#define CLIENTCOUNT 1

char* g_strIp = "127.0.0.1";
unsigned int g_dwPort = 20000;
bool g_bRun = true;

static FxSession* g_sSessions[CLIENTCOUNT] = { 0 };

class TestTimer : public IFxTimer
{
	virtual bool OnTimer(double fSecond)
	{
		for (int i = 0; i < CLIENTCOUNT; ++i)
		{
			if (!g_sSessions[i]->GetConnection())
			{
				continue;
			}
			if (!g_sSessions[i]->IsConnected())
			{
				continue;
			}
			g_sSessions[i]->ForceSend();
		}
		GetTimeHandler()->AddDelayTimer(0.01, this);
		return true;
	}
};
TestTimer g_sTimer;

class TestTimer1 : public IFxTimer
{
public:
	TestTimer1()
	{
		m_i = 0;
	}
	virtual bool OnTimer(double fSecond)
	{
		int nLen = rand() % 256;
		nLen += 768;
		bool bSended = true;
		for (int i = 0; i < CLIENTCOUNT; ++i)
		{
			sprintf(szMsg, "%s....%d......%d.....%d", GetExePath(), g_sSessions[i]->GetRemotePort(), m_i, nLen);
			if (!g_sSessions[i]->GetConnection())
			{
				continue;
			}
			if (!g_sSessions[i]->IsConnected())
			{
				continue;
			}
			if(!g_sSessions[i]->Send(szMsg, 512))
			{
				if (!g_sSessions[i]->IsConnected())
				{
					LogExe(LogLv_Error, "error!!!!!!!!!!!!!!!!!!!!!!!!");
					g_bRun = false;
					break;
				}
				bSended = false;
			}
		}
		if(bSended)
		{
			++m_i;
		}
		GetTimeHandler()->AddDelayTimer(0.01, this);
		return true;
	}
	char szMsg[1024];
	int m_i;
};
TestTimer1 g_sTimer1;

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
	//if (!LogThread::CreateInstance())
	//{
	//	return 0;
	//}

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
	if (!CLuaEngine::Instance()->Reload(WORK_PATH))
	{
		return 0;
	}
	// must defined before goto
	IFxNet* pNet = NULL;
	UINT32 dwIP = 0;
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
	//if (!LogThread::Instance()->Init())
	//{
	//	g_bRun = false;
	//	goto STOP;
	//}

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
		g_sSessions[i] = oSessionFactory.CreateSession();
		pNet->UdpConnect(g_sSessions[i], dwIP, g_dwPort, true);
	}

	GetTimeHandler()->AddDelayTimer(0.011f, &g_sTimer);
	GetTimeHandler()->AddDelayTimer(0.08f, &g_sTimer1);

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		//for (int i = 0; i < CLIENTCOUNT; ++i)
		//{
		//	if (g_sSessions[i]->IsConnected())
		//	{
		//		sprintf(szMsg, "%s....%d......%d", GetExePath(), g_sSessions[i]->GetRemotePort(), j);
		//		while(!g_sSessions[i]->Send(szMsg, 1024))
		//		{
		//			if (!g_sSessions[i]->IsConnected())
		//			{
		//				LogExe(LogLv_Error, "error!!!!!!!!!!!!!!!!!!!!!!!!");
		//				g_bRun = false;
		//				break;
		//			}
		//			GetTimeHandler()->Run();
		//			FxSleep(10);
		//			//pSession->Close();
		//		}
		//		//else
		//		//{
		//		//	LogExe(LogLv_Debug, "send : %s", szMsg);
		//		//	//++i;
		//		//}
		//		FxSleep(1);
		//	}
		//	else
		//	{
		//		if (!g_sSessions[i]->IsConnecting())
		//		{
		//			g_sSessions[i]->Reconnect();
		//		}
		//	}
		//}
		FxSleep(10);
		++j;
	}
	for (int i = 0; i < CLIENTCOUNT; ++i)
	{
		g_sSessions[i]->Close();
	}
	FxSleep(10);
	pNet->Run(0xffffffff);
	FxSleep(10);
	pNet->Release();
	return 0;
STOP:
	printf("error!!!!!!!!\n");
	//LogThread::Instance()->Stop();
}
