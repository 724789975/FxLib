#include "SocketSession.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"

#include <signal.h>
#include "gflags/gflags.h"

unsigned int g_dwPort = 20000;
bool g_bRun = true;

DEFINE_uint32(tcp_port, 20001, "tcp linten port");
DEFINE_uint32(udp_port, 20000, "udp linten port");

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
	IFxListenSocket* pTcpListenSocket = NULL;
	IFxListenSocket* pUdpListenSocket = NULL;
	UINT16 wPort = 0;

	if (!GetTimeHandler()->Init())
	{
		g_bRun = false;
		goto STOP;
	}
	GetTimeHandler()->Run();

	if (!CSessionFactory::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}
	CSessionFactory::Instance()->Init();

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

	wPort = FLAGS_tcp_port;
	pTcpListenSocket = pNet->Listen(CWebSocketSessionFactory::Instance(), SLT_WebSocket, 0, wPort);
	if(pTcpListenSocket == NULL)
	{
		g_bRun = false;
		goto STOP;
	}

	wPort = FLAGS_udp_port;
	pUdpListenSocket = pNet->Listen(CSessionFactory::Instance(), SLT_Http, 0, wPort);
	if(pUdpListenSocket == NULL)
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
	pTcpListenSocket->StopListen();
	pUdpListenSocket->StopListen();
	pTcpListenSocket->Close();
	pUdpListenSocket->Close();
	for (std::set<FxSession*>::iterator it = CSessionFactory::Instance()->m_setSessions.begin();
		it != CSessionFactory::Instance()->m_setSessions.end(); ++it)
	{
		(*it)->Close();
	}

	while (CSessionFactory::Instance()->m_setSessions.size())
	{
		pNet->Run(0xffffffff);
		FxSleep(10);
	}
	FxSleep(10);
	pNet->Release();
	return 0;
STOP:
	printf("error!!!!!!!!\n");
	//LogThread::Instance()->Stop();
}
