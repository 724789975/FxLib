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

	// must define before goto
	IFxNet* pNet = NULL;
	IFxListenSocket* pListenSocket = NULL;

	if (!GetTimeHandler()->Init())
	{
		g_bRun = false;
		goto STOP;
	}
	GetTimeHandler()->Run();

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

	UINT16 wPort = FLAGS_port;
	pListenSocket = pNet->Listen(CWebSocketSessionFactory::Instance(), SLT_WebSocket, 0, wPort);

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
}
