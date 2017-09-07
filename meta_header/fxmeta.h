#ifndef __FXBASE_H__
#define __FXBASE_H__

#include <stdarg.h>
#include <stdio.h>
#include "singleton.h"
#include "fxtimer.h"
#include "thread.h"
#include "log_thread.h"
#include "redef_assert.h"

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // WIN32

typedef signed char			INT8;
typedef unsigned char		UINT8;
typedef signed short		INT16;
typedef unsigned short		UINT16;
typedef signed int			INT32;
typedef unsigned int		UINT32;
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
#ifndef NULL
#define NULL 0
#endif

class ListDirAndLoadFile
{
public:
	virtual ~ListDirAndLoadFile()
	{
	}
	virtual bool LoadFile(const char* pFileName) = 0;
};

void ListDir(const char* pDirName, ListDirAndLoadFile* pListDirAndLoadFile);

class IFxMeta
{
public:
	virtual ~IFxMeta(void)
	{
	}

	virtual void AddRef(void) = 0;

	virtual UINT32 QueryRef(void) = 0;

	virtual void Release(void) = 0;

	virtual const char * GetModuleName(void) = 0;
};

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
		"LogLv_Error	",
		"LogLv_Critical	",
		"LogLv_Warn		",
		"LogLv_Info		",
		"LogLv_Debug	",
		"LogLv_Debug1	",
		"LogLv_Debug2	",
		"LogLv_Debug3	",
};

FILE* GetLogFile();

void PrintTrace(char* strTrace);

bool Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...);

#define LogFun(eLogType, eLevel, strFmt, ...)\
{\
	{\
		char strLog[2048] = {0};\
		if(eLevel < LogLv_Count)\
		{\
			int nLenStr = 0;\
			nLenStr += sprintf(strLog + nLenStr, "%s:%d ", GetTimeHandler()->GetTimeStr(), GetTimeHandler()->GetTimeSeq());\
			nLenStr += sprintf(strLog + nLenStr, "%s", LogLevelString[eLevel]);\
			nLenStr += sprintf(strLog + nLenStr, "[%s,%d,%s] ", __FILE__, __LINE__, __FUNCTION__);\
			nLenStr += sprintf(strLog + nLenStr, strFmt, ##__VA_ARGS__);\
			nLenStr += sprintf(strLog + nLenStr, "%s", "\n");\
			if(eLevel == LogLv_Error)\
			{\
				PrintTrace(strLog + nLenStr);\
			}\
			LogThread::Instance()->ReadLog(eLogType, strLog);\
		}\
		else\
		{\
			printf("error log fun !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
		}\
	}\
}

// can be used only in main thread
#define LogExe(eLevel, strFmt, ...)\
{\
	{\
		char strLog[2048] = {0};\
		FILE* pFile = GetLogFile();\
		Assert(pFile);\
		if (pFile)\
		{\
			if((eLevel < LogLv_Count))\
			{\
				int nLenStr = 0;\
				nLenStr += sprintf(strLog + nLenStr, "%s:%d ", GetTimeHandler()->GetTimeStr(), GetTimeHandler()->GetTimeSeq());\
				nLenStr += sprintf(strLog + nLenStr, "%s", LogLevelString[eLevel]);\
				nLenStr += sprintf(strLog + nLenStr, "[%s,%d,%s] ", __FILE__, __LINE__, __FUNCTION__);\
				nLenStr += sprintf(strLog + nLenStr, strFmt, ##__VA_ARGS__);\
				nLenStr += sprintf(strLog + nLenStr, "%s", "\n");\
				if(eLevel == LogLv_Error)\
				{\
					PrintTrace(strLog + nLenStr);\
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

inline const char* GetSeparator()
{
#ifdef WIN32
	return "\\";
#else
	return "/";
#endif // WIN32
}

#ifdef WIN32
#define Access _access
#else
#define Access access
#endif

//#define ThreadLog(eLevel, pFile, dwThreadId, strFmt, ...)
#define ThreadLog(eLevel, pFile, szLogFile, strFmt, ...)\
{\
	{\
		char strLog[2048] = {0};\
		if (pFile == NULL)\
		{\
			pFile = fopen(szLogFile, "a+");\
		}\
		if (Access(szLogFile, 0) == -1)\
		{\
			fclose(pFile);\
			pFile = fopen(szLogFile, "a+");\
		}\
		if (pFile)\
		{\
			if((eLevel < LogLv_Count))\
			{\
				int nLenStr = 0;\
				nLenStr += sprintf(strLog + nLenStr, "%s:%d ", GetTimeHandler()->GetTimeStr(), GetTimeHandler()->GetTimeSeq());\
				nLenStr += sprintf(strLog + nLenStr, "%s", LogLevelString[eLevel]);\
				nLenStr += sprintf(strLog + nLenStr, "[%s,%d,%s] ", __FILE__, __LINE__, __FUNCTION__);\
				nLenStr += sprintf(strLog + nLenStr, strFmt, ##__VA_ARGS__);\
				nLenStr += sprintf(strLog + nLenStr, "%s", "\n");\
				if(eLevel == LogLv_Error)\
				{\
					PrintTrace(strLog + nLenStr);\
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

IFxLock* FxCreateThreadLock();

IFxLock* FxCreateThreadFakeLock();

void FxSleep(UINT32 dwMilliseconds);

char* GetExePath();
char* GetExeName();

#endif
