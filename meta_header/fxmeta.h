#ifndef __FXBASE_H__
#define __FXBASE_H__

#include <stdarg.h>
#include <stdio.h>
#include "singleton.h"
#include "fxtimer.h"

typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;
#ifndef NULL
#define NULL 0
#endif

typedef struct
{
	UINT32 m_dwSec;		///< seconds
	UINT32 m_dwUsec;	///< microsecond(1/1,000,000 second)
} STimeVal;

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

//#define LOGLV_NONE			0x0000
//#define LOGLV_DEBUG			0x0001
//#define LOGLV_INFO			(0x0001<< 1)
//#define LOGLV_WARN			(0x0001<< 2)
//#define LOGLV_CRITICAL		(0x0001<< 3)

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

#define HALF_GIGA	512*1024*1024

FILE* GetLogFile(const char* strPath);

char* PrintTrace();

bool Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...);

#define LogScreen(eLevel, strFmt, ...)\
{\
	{\
		if(eLevel < LogLv_Count)\
		{\
			char Temp[2048] = { 0 };\
			int nLenStr = 0;\
			nLenStr = sprintf(Temp, "%s ", GetTimeHandler()->GetTimeStr()); \
			nLenStr += sprintf(Temp + nLenStr, "%s ", LogLevelString[eLevel]); \
			nLenStr += sprintf(Temp + nLenStr, "[%s,%s,%d] ", __FILE__, __FUNCTION__, __LINE__); \
			nLenStr += sprintf(Temp + nLenStr, strFmt, ##__VA_ARGS__);\
			nLenStr += sprintf(Temp + nLenStr, "%s", "\n"); \
			if(eLevel == LogLv_Error)\
			{\
				sprintf(Temp + nLenStr, "%s", PrintTrace());\
			}\
			printf("%s", Temp);\
		}\
	}\
}

#define LogFile(eLevel, strFmt, ...)\
{\
	{\
		FILE* pFile = GetLogFile("log.txt");\
		if(pFile && (eLevel < LogLv_Count))\
		{\
			char Temp[1024] = { 0 };\
			int nLenStr = 0;\
			nLenStr = sprintf(Temp, "%s ", GetTimeHandler()->GetTimeStr()); \
			nLenStr += sprintf(Temp + nLenStr, "%s ", LogLevelString[eLevel]); \
			nLenStr += sprintf(Temp + nLenStr, "[%s,%s,%d] ", __FILE__, __FUNCTION__, __LINE__); \
			nLenStr += sprintf(Temp + nLenStr, strFmt, ##__VA_ARGS__);\
			nLenStr += sprintf(Temp + nLenStr, "%s ", "\n"); \
			if(eLevel == LogLv_Error)\
			{\
				sprintf(Temp + nLenStr, "%s", PrintTrace());\
			}\
			int ret = 0;\
			ret = fprintf(pFile, "%s ", Temp);\
			if(ret <= 0)\
			{\
				printf("error put string !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");\
			}\
		}\
		else\
		{\
			printf("error log file!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
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

class IFxThread
{
public:
	virtual ~IFxThread()
	{
	}

	virtual void ThrdFunc() = 0;

	virtual void Stop() = 0;
};

class IFxThreadHandler
{
public:
	virtual ~IFxThreadHandler()
	{
	}

	virtual void Stop(void) = 0;

	virtual bool Kill(UINT32 dwExitCode) = 0;

	virtual bool WaitFor(UINT32 dwWaitTime = FX_INFINITE) = 0;

	virtual UINT32 GetThreadId(void) = 0;

	virtual IFxThread* GetThread(void) = 0;

	virtual void Release(void) = 0;

	bool IsStop(void)
	{
		return m_bIsStop;
	}

#ifdef WIN32
	void* GetHandle(void)
	{	return m_hHandle;}
protected:
	void* m_hHandle;		// ????????? ?????? NULL ???? INVALID_HANDLE_VALUE??
#endif // WIN32

protected:
	bool m_bIsStop;
};

IFxLock* FxCreateThreadLock();

IFxLock* FxCreateThreadFakeLock();

IFxThreadHandler* FxCreateThreadHandler(IFxThread* poThread, bool bNeedWaitfor);

void FxSleep(UINT32 dwMilliseconds);

char* GetExePath();

#endif
