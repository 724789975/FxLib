#include "SocketSession.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"

#include <signal.h>
#include "gflags/gflags.h"

unsigned int g_dwPort = 20000;
bool g_bRun = true;

DEFINE_uint32(port, 20000, "linten port");

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
	//if (!LogThread::CreateInstance())
	//{
	//	return 0;
	//}

	// must define before goto
	IFxNet* pNet = NULL;
	IFxListenSocket* pListenSocket = NULL;

	//if (!CLuaEngine::Instance()->CommandLineFunction(argv, argc))
	//{
	//	g_bRun = false;
	//	goto STOP;
	//}
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

	if (!CWebSocketSessionFactory::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}
	CWebSocketSessionFactory::Instance()->Init();
	CChatManagerSession::CreateInstance();
	pNet = FxNetGetModule();
	if (!pNet)
	{
		g_bRun = false;
		goto STOP;
	}
	//----------------------order can't change end-----------------------//

	//SDBConnInfo oInfo;
	//memset(&oInfo, 0, sizeof(oInfo));
	//oInfo.m_dwDBId = 0;
	//strcpy(oInfo.m_stAccount.m_szCharactSet, "utf8");
	//strcpy(oInfo.m_stAccount.m_szDBName, "jianghu");
	//strcpy(oInfo.m_stAccount.m_szHostName, "192.168.5.6");
	//strcpy(oInfo.m_stAccount.m_szLoginName, "root");
	//strcpy(oInfo.m_stAccount.m_szLoginPwd, "test");
	//oInfo.m_stAccount.m_wConnPort = 3306;
	//if (FxDBGetModule()->Open(oInfo))
	//{
	//	LogFun(LT_Screen, LogLv_Info, "%s", "db connected~~~~");
	//}

	pListenSocket = pNet->Listen(CWebSocketSessionFactory::Instance(), SLT_WebSocket, 0, FLAGS_port);

	pNet->TcpConnect(CChatManagerSession::Instance(), inet_addr("127.0.0.1"), 13001, true);
	if(pListenSocket == NULL)
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
	pListenSocket->StopListen();
	pListenSocket->Close();
	for (std::set<FxSession*>::iterator it = CWebSocketSessionFactory::Instance()->m_setSessions.begin();
		it != CWebSocketSessionFactory::Instance()->m_setSessions.end(); ++it)
	{
		(*it)->Close();
	}

	while (CWebSocketSessionFactory::Instance()->m_setSessions.size())
	{
		pNet->Run(0xffffffff);
		FxSleep(10);
	}
	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");
	//LogThread::Instance()->Stop();
}
