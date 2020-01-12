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

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // WIN32


namespace Utility
{
	int						GetPid();
	char*					GetExePath();
	char*					GetExeName();
	const char*				GetSeparator();
	FILE*					GetLogFile();
	void					PrintTrace(char* strTrace, int dwLen);
	bool					Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...);
	void					FxSleep(unsigned int dwMilliseconds);
	const char*				GetOsInfo();

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
