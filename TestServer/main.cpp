#include "SocketSession.h"
#include "lua_engine.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"

int main()
{
	LogThread::CreateInstance();
	LogThread::Instance()->Init();

	CLuaEngine::CreateInstance();

	CLuaEngine::Instance()->Reload();

	GetTimeHandler()->Init();
	GetTimeHandler()->Run();

	CSessionFactory::CreateInstance();
	CSessionFactory::Instance()->Init();
	IFxNet* pNet = FxNetGetModule();

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
	//	LogScreen("db connected~~~~");
	//}

	pNet->Listen(CSessionFactory::Instance(), 0, 0, 12000);

	while (true)
	{
		GetTimeHandler()->Run();
		pNet->Run(0xffffffff);
		//LogScreen(PrintTrace());
		//LogFile(PrintTrace());
		FxSleep(1);
	}
}
