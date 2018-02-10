#include "SocketSession.h"
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

int main(int argc, char **argv)
{
	//----------------------order can't change begin-----------------------//
	gflags::SetUsageMessage("ChatServer");
	gflags::ParseCommandLineFlags(&argc, &argv, false);
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);
	//if (!LogThread::CreateInstance())
	//{
	//	return 0;
	//}

	// must define before goto
	IFxNet* pNet = NULL;

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();
	//if (!LogThread::Instance()->Init())
	//{
	//	g_bRun = false;
	//	goto STOP;
	//}

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
	oInfo.m_dwDBId = FLAGS_db_id;
	strcpy(oInfo.m_stAccount.m_szCharactSet, FLAGS_character_set.c_str());
	strcpy(oInfo.m_stAccount.m_szDBName, FLAGS_db_name.c_str());
	strcpy(oInfo.m_stAccount.m_szHostName, FLAGS_host_name.c_str());
	strcpy(oInfo.m_stAccount.m_szLoginName, FLAGS_login_name.c_str());
	strcpy(oInfo.m_stAccount.m_szLoginPwd, FLAGS_login_pwd.c_str());
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

	UINT16 w1 = FLAGS_chat_session_port, w2 = FLAGS_chat_web_socket_session, w3 = FLAGS_chat_server_session_port;
	if (!ChatServer::Instance()->Init(FLAGS_chat_server_ip, w1, w2, w3))
	{
		g_bRun = false;
		goto STOP;
	}

	while (g_bRun)
	{
		GetTimeHandler()->Run();
		FxDBGetModule()->Run();
		pNet->Run(0xffffffff);
		//LogFun(LT_Screen, LogLv_Info, "%s", PrintTrace());
		FxSleep(1);
	}
	ChatServer::Instance()->Close();
	FxSleep(10);
	pNet->Release();
STOP:
	printf("error!!!!!!!!\n");
	//LogThread::Instance()->Stop();
}
