#include "fxtimer.h"
#include "fxredis.h"
#include "fxmeta.h"
#include "gflags/gflags.h"

#include <signal.h>

DEFINE_string(chat_server_ip, "127.0.0.1", "Chat Server Ip");
DEFINE_uint32(chat_session_port, 20000, "Chat Session Port");
DEFINE_uint32(chat_web_socket_session, 20001, "Web Socket Session Port");
DEFINE_uint32(chat_server_session_port, 20002, "Chat Server Session Port");
DEFINE_string(chat_server_manager_ip, "127.0.0.1", "Chat Server Manager Ip");
DEFINE_uint32(chat_server_manager_port, 13000, "Chat Server Manager Port");

DEFINE_uint32(db_id, 0, "DB Id");
DEFINE_uint32(db_port, 3306, "DB Id");
DEFINE_string(character_set, "utf8", "Character Set");
DEFINE_string(db_name, "chat", "DB Name");
DEFINE_string(host_name, "127.0.0.1", "Host Name");
DEFINE_string(login_name, "test", "Login Name");
DEFINE_string(login_pwd, "test", "DB PassWord");

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
	gflags::SetUsageMessage("ChatServer");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	//----------------------order can't change end-----------------------//

	if (!FxRedisGetModule()->Open("127.0.0.1", 6379, 0))
	{
		LogExe(LogLv_Info, "%s", "db connected failed~~~~");
		goto STOP;
	}

	FxRedisGetModule()->AddQuery(&t1);

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
