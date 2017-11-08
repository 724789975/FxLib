#include "lua_engine.h"
#include "tolua++.h"

#include "tarray.h"
#include <vector>

#include "fxtimer.h"

#include <signal.h>

//#include<getopt.h>

class TestTimer: public IFxTimer
{
	virtual bool OnTimer(float fSecond)
	{
		LogFun(LT_File | LT_Screen, LogLv_Error, "%f", fSecond);
		return true;
	}
};

int a[10] =
{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
Point p[10] =
{
{ 0, 1 },
{ 1, 2 },
{ 2, 3 },
{ 3, 4 },
{ 4, 5 },
{ 5, 6 },
{ 6, 7 },
{ 7, 8 },
{ 8, 9 },
{ 9, 10 } };
Point* pp[10];

int ma[10];
Point mp[10];
Point* mpp[10];

Array array;
Array* parray = &array;

//struct option long_options[] =
//{
//{ "add", 1, 0, 0 },
//{ "append", 0, 0, 0 },
//{ "delete", 1, 0, 0 },
//{ "create", 0, 0, 'c' },
//{ "file", 1, 0, 0 },
//{ 0, 0, 0, 0 } };

bool g_bRun = true;

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
	signal(SIGINT, EndFun);
	signal(SIGTERM, EndFun);
	LogThread::CreateInstance();
	LogThread::Instance()->Init();

	int tolua_tarray_open(lua_State*);

	CLuaEngine::CreateInstance();
	std::vector<ToluaFunctionOpen*> vecFunctions;
	vecFunctions.push_back(tolua_tarray_open);
	CLuaEngine::Instance()->Init(vecFunctions);
//	oEngine.Init();
	int i;

	for (i = 0; i < 10; ++i)
	{
		pp[i] = &p[i];

		ma[i] = a[i];
		mp[i] = p[i];
		mpp[i] = pp[i];

		array.a[i] = a[i];
		array.p[i] = p[i];
		array.pp[i] = pp[i];
	}

	CLuaEngine::Instance()->Reload(WORK_PATH);

	//while (1)
	//{
	//	/*getopt_long stores the option index here. */
	//	int option_index = 0;

	//	int c = getopt_long(argc, argv, "abc:d:", long_options, &option_index);

	//	/*Detect the end of the options. */
	//	if (c == -1)
	//	{
	//		break;
	//	}
	//	char strCmd[64] =
	//	{ 0 };

	//	if (c == 0)
	//	{
	//		sprintf(strCmd, "%s", long_options[option_index].name);
	//	}
	//	else
	//	{
	//		sprintf(strCmd, "%c", c);
	//	}
	//	CLuaEngine::Instance()->CallVoidFunction<char*, char*>("Option", strCmd,
	//			optarg);
	//}

	int aaa = (int)CLuaEngine::Instance()->CallNumberFunction<int, char>("add", 3,
			4);

	//printf("%s",
	//		CLuaEngine::Instance()->CallStringFunction<int, char*>("test1", 3,
	//				"!!!!!!!!!11"));

	Point* p1 = CLuaEngine::Instance()->CallUserFunction<Point, Point*, int>(
			"test", &p[0], 4);

	if (!CLuaEngine::Instance()->CommandLineFunction(argv, argc))
	{
		return 0;
	}

	GetTimeHandler()->Init();
	GetTimeHandler()->Run();

	TestTimer oTimer;
	GetTimeHandler()->AddDelayTimer(10, &oTimer);
	GetTimeHandler()->AddEveryFewMinuteTimer(5, &oTimer);
	while (g_bRun)
	{
		GetTimeHandler()->Run();
		FxSleep(1000);
	}
	LogThread::Instance()->Stop();
	return 0;
}
