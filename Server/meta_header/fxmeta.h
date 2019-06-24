#ifndef __FXBASE_H__
#define __FXBASE_H__

#include <stdarg.h>
#include <stdio.h>
#include "singleton.h"
#include "fxtimer.h"
#include "thread.h"
#include "log_thread.h"
#include "redef_assert.h"
#include "defines.h"


#include "utility.h"

using namespace Utility;

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // WIN32

//typedef signed char			INT8;
//typedef unsigned char		unsigned char;
//typedef signed short		short;
//typedef unsigned short		unsigned short;
//typedef signed int			int;
//typedef unsigned int		unsigned int;
//typedef signed long long	long long;
//typedef unsigned long long	unsigned long long;
#ifndef NULL
#define NULL 0
#endif

enum LOGLEVEL
{
	LogLv_Error,
	LogLv_Critical,
	LogLv_Warn,
	LogLv_Info,
	LogLv_Debug,
	LogLv_Debug1,
	LogLv_Debug2,
	LogLv_Debug3,
	LogLv_Count,
};

static const char* LogLevelString[LogLv_Count] =
{
		"LogLv_Error    ",
		"LogLv_Critical ",
		"LogLv_Warn     ",
		"LogLv_Info     ",
		"LogLv_Debug    ",
		"LogLv_Debug1   ",
		"LogLv_Debug2   ",
		"LogLv_Debug3   ",
};


// can be used only in main thread
#define LogExe(eLevel, strFmt, ...)\
{\
	{\
		char strLog[4096] = {0};\
		FILE* pFile = GetLogFile();\
		Assert(pFile);\
		if (pFile)\
		{\
			if((eLevel < LogLv_Count))\
			{\
				int nLenStr = 0;\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s.%d\t", GetTimeHandler()->GetTimeStr(), GetTimeHandler()->GetTimeSeq());\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s", LogLevelString[eLevel]);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "[%s,%d,%s] ", __FILE__, __LINE__, __FUNCTION__);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, strFmt, ##__VA_ARGS__);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s", "\n");\
				if(eLevel == LogLv_Error)\
				{\
					PrintTrace(strLog + nLenStr, 2048 - nLenStr);\
				}\
				fprintf(pFile, "%s", strLog);\
				printf("%s", strLog);\
			}\
			else\
			{\
				printf("error log fun !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
			}\
		}\
		else\
		{\
			printf("error pFile !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
		}\
	}\
}





//#define ThreadLog(eLevel, pFile, dwThreadId, strFmt, ...)
#define ThreadLog(eLevel, pFile, strFmt, ...)\
{\
	{\
		char strLog[4096] = {0};\
		if (pFile)\
		{\
			if((eLevel < LogLv_Count))\
			{\
				int nLenStr = 0;\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s.%d\t", GetTimeHandler()->GetTimeStr(), GetTimeHandler()->GetTimeSeq());\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s ", LogLevelString[eLevel]);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "[%s,%d,%s] ", __FILE__, __LINE__, __FUNCTION__);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, strFmt, ##__VA_ARGS__);\
				nLenStr += string_sprintf(strLog + nLenStr, 4096 - nLenStr - 1, "%s", "\n");\
				if(eLevel == LogLv_Error)\
				{\
					PrintTrace(strLog + nLenStr, 2048 - nLenStr);\
				}\
				fprintf(pFile, "%s", strLog);\
			}\
			else\
			{\
				printf("error log fun !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
			}\
		}\
		else\
		{\
			printf("error pFile !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
		}\
	}\
}

class IFxLock
{
public:
	virtual ~IFxLock()
	{
	}

	virtual void Lock() = 0;

	virtual void UnLock() = 0;

	virtual void Release() = 0;
};

class FxLockImp
{
public:
	explicit FxLockImp(IFxLock* locker) :
			m_pLocker(locker)
	{
		if (NULL != m_pLocker)
		{
			m_pLocker->Lock();
		}
	}
	~FxLockImp()
	{
		if (NULL != m_pLocker)
		{
			m_pLocker->UnLock();
		}
		m_pLocker = NULL;
	}

private:
	IFxLock* m_pLocker;
};

#define FX_INFINITE			0xffffffff


#endif
