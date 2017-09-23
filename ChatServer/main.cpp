#include "SocketSession.h"
#include "lua_engine.h"
#include "fxtimer.h"
#include "fxdb.h"
#include "fxmeta.h"
#include "gflags/gflags.h"
#include "ChatServer.h"

#include <signal.h>

DEFINE_string(chat_server_ip, "127.0.0.1", "Chat Server Ip");
DEFINE_uint32(chat_session_port, 20000, "Chat Session Port");
DEFINE_uint32(chat_web_socket_session, 20001, "Web Socket Session Port");
DEFINE_uint32(chat_server_session_port, 20002, "Chat Server Session Port");
DEFINE_string(chat_server_manager_ip, "127.0.0.1", "Chat Server Manager Ip");
DEFINE_uint32(chat_server_manager_port, 13000, "Chat Server Manager Port");

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

int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("ChatServer");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
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
	// must define before goto
	IFxNet* pNet = NULL;

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	if (!LogThread::Instance()->Init())
	{
		g_bRun = false;
		goto STOP;
	}

	if (!ChatServer::CreateInstance())
	{
		g_bRun = false;
		goto STOP;
	}
	pNet = FxNetGetModule();
	if (!pNet)
	{
		g_bRun = false;
		goto STOP;
	}
	//----------------------order can't change end-----------------------//

	SDBConnInfo oInfo;
	memset(&oInfo, 0, sizeof(oInfo));
	oInfo.m_dwDBId = 0;
	strcpy(oInfo.m_stAccount.m_szCharactSet, "utf8");
	strcpy(oInfo.m_stAccount.m_szDBName, "chat");
	strcpy(oInfo.m_stAccount.m_szHostName, "127.0.0.1");
	strcpy(oInfo.m_stAccount.m_szLoginName, "test");
	strcpy(oInfo.m_stAccount.m_szLoginPwd, "test");
	oInfo.m_stAccount.m_wConnPort = 3306;
	if (!FxDBGetModule()->Open(oInfo))
	{
		LogExe(LogLv_Info, "%s", "db connected failed~~~~");
		goto STOP;
	}

	if (pNet->TcpConnect(&ChatServer::Instance()->GetChatManagerSession(), inet_addr(FLAGS_chat_server_manager_ip.c_str()), FLAGS_chat_server_manager_port) == INVALID_SOCKET)
	{
		g_bRun = false;
		goto STOP;
	}

	if (!ChatServer::Instance()->Init(FLAGS_chat_server_ip, FLAGS_chat_session_port, FLAGS_chat_web_socket_session, FLAGS_chat_server_session_port))
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
	ChatServer::Instance()->Close();
	FxSleep(10);
	pNet->Release();
STOP:
	LogThread::Instance()->Stop();
}
