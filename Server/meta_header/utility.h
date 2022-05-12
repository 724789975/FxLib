#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdarg.h>
#include <stdio.h>
#include "singleton.h"
#include "fxtimer.h"
#include "thread.h"
#include "log_thread.h"
#include "redef_assert.h"
#include "defines.h"

#ifdef _WIN32
#include <io.h>
#include <WinSock2.h>
#include <Windows.h>
#else
#include <unistd.h>
#endif // _WIN32


namespace Utility
{
	int						GetPid();
	int						GetTid();
	char*					GetExePath();
	char*					GetExeName();
	const char*				GetSeparator();
	FILE*					GetLogFile();
	void					PrintTrace(char* strTrace, int dwLen);
	bool					Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...);
	void					FxSleep(unsigned int dwMilliseconds);
	const char*				GetOsInfo();

	struct ProcCpuInfo
	{
		unsigned long long qwUTime = 0;
		unsigned long long qwSTime = 0;
	};

	struct SysCpuInfo
	{
		unsigned long long qwUTime = 0;
		unsigned long long qwSTime = 0;
		unsigned long long qwNice = 0;
		unsigned long long qwIdle = 0;

		unsigned long long qwIOWait = 0;
		unsigned long long qwIrq = 0;
		unsigned long long qwSoftIrq = 0;
		unsigned long long qwSteal = 0;
	};

	struct CpuSample
	{
		ProcCpuInfo oProcCpuInfo;
		SysCpuInfo oSysCpuInfo;
	};

	int							GetCpuInfo();
	int							GetSysCpuInfo(SysCpuInfo& orefSysCpuInfo);
	int							GetCpuCoreNumber();
	void						SampleCupInfo(CpuSample& oSample);
	int							CalcCpuUsage(const CpuSample& refPrev, const CpuSample& refCurr);

	class ListDirAndLoadFile
	{
	public:
		virtual ~ListDirAndLoadFile()
		{
		}
		virtual bool operator()(const char* pFileName) = 0;
	};
	void ListDir(const char* pDirName, ListDirAndLoadFile& refLoadFile);
	
}

//#define LogOutHeader(eLevel, os1)\
//{\
//	if((eLevel < LogLv_Count)) \
//	{\
//		if(os1) {(*os1) << GetTimeHandler()->GetTimeStr() << "." << GetTimeHandler()->GetTimeSeq() << "\t";} \ 
//if (os1) { (*os1) << LogLevelString[eLevel] << "[" << __FILE__ << "," << __LINE__ << "," << __FUNCTION__; } \
//	}\
//}

#endif	//__UTILITY_H__
