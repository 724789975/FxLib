#include "fxtimer.h"
#include "fxredis.h"
#include "fxmeta.h"
#include "gflags/gflags.h"

#include <signal.h>

bool g_bRun = true;

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

class RedisTest : public IRedisQuery
{
public:
	RedisTest() {}
	~RedisTest() {}

	virtual int					GetDBId(void) { return 0; }
	virtual void				OnQuery(IRedisConnection *poDBConnection)
	{
		poDBConnection->Query("get i", &m_pReader);
	}
	virtual void				OnResult(void)
	{
		std::string szRet;
		m_pReader->GetValue(szRet);
	}
	virtual void				Release(void)
	{
		m_pReader->Release();
	}

private:
	IRedisDataReader* m_pReader;
};

RedisTest t1;
int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("RedisTest");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	//----------------------order can't change end-----------------------//

	if (!FxRedisGetModule()->Open("127.0.0.1", 16379, "1", 0))
	{
		LogExe(LogLv_Info, "%s", "db connected failed~~~~");
		goto STOP;
	}

	//FxRedisGetModule()->AddQuery(&t1);
	FxRedisGetModule()->QueryDirect(&t1);

	//t1.OnQuery()

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		FxRedisGetModule()->Run();
		//LogFun(LT_Screen, LogLv_Info, "%s", PrintTrace());
		FxSleep(1);
	}
STOP:
	FxSleep(1);
}
