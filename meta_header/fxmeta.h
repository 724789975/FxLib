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

#define LOGLV_NONE			0x0000
#define LOGLV_DEBUG			0x0001
#define LOGLV_INFO			(0x0001<< 1)
#define LOGLV_WARN			(0x0001<< 2)
#define LOGLV_CRITICAL		(0x0001<< 3)

#define HALF_GIGA	512*1024*1024

FILE* GetLogFile(const char* strPath);

bool Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...);

#define LogScreen(strFmt, ...)\
{\
	char Temp[1024] = { 0 };\
	int nLen = 0;\
	nLen = sprintf(Temp, "%s ", GetTimeHandler()->GetTimeStr()); \
	nLen += sprintf(Temp + nLen, "[%s,%s,%d] ", __FILE__, __FUNCTION__, __LINE__); \
	Log(Temp + nLen, 1024 - nLen, strFmt, ##__VA_ARGS__); \
	printf("%s", Temp);\
}

#define LogFile(strFmt, ...)\
{\
	FILE* pFile = GetLogFile("log.txt");\
	if(pFile)\
	{\
		int ret = 0;\
		ret = fprintf(pFile, "%s ", GetTimeHandler()->GetTimeStr());\
		if(ret <= 0)\
		{\
			printf("error put string !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");\
		}\
		fprintf(pFile, "[%s,%s,%d] ", __FILE__, __FUNCTION__, __LINE__); \
		if(ret <= 0)\
		{\
			printf("error put string !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");\
		}\
		fprintf(pFile, strFmt,  ##__VA_ARGS__);\
		if(ret <= 0)\
		{\
			printf("error put string !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");\
		}\
		fprintf(pFile, "%s", "\n");\
		if(ret <= 0)\
		{\
			printf("error put string !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");\
		}\
	}\
	else\
	{\
		printf("error log file!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
	}\
}

char* PrintTrace();

//class IFxLogger
//{
//public:
//	virtual	~IFxLogger(){}
//
//	/*
//	Function:		Initialize.
//	Param:
//		pszName:		Logfile name.
//		dwMaxSize:		Max file size.
//	*/
//	virtual	bool Init(const char* pszName, bool bThrdSafe = true, bool bTimeHeader = true, UINT32 dwMaxSize = HALF_GIGA) = 0;
//
//	virtual	bool LogText(const char *pszLog) = 0;
//
//	virtual	bool LogBinary(const UINT8 *pLog, UINT32 dwLen) = 0;
//
//	virtual	 UINT32 GetLogBlockSize() = 0;
//
//	virtual	void Release(void) = 0;
//};

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

//class IFxLogCenterReporter
//{
//public:
//	virtual	~IFxLogCenterReporter() {}
//
//	virtual	bool			Init(UINT16 wPort, char* szBPID) = 0;
//	virtual void			Log(char* szApiName, char* szJsonData) = 0;
//};

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

	//virtual void			Resume(void) = 0;

	//virtual void			Suspend() = 0;

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

	//virtual bool			Start(void){}

#ifdef WIN32
	void* GetHandle(void)
	{	return m_hHandle;}
protected:
	void* m_hHandle;		// ????????? ?????? NULL ???? INVALID_HANDLE_VALUE??
#endif // WIN32

protected:
	bool m_bIsStop;
};

//class IFxTimer
//{
//public:
//	virtual ~IFxTimer(){}
//
//	virtual bool			OnTimer(int nMicroSecond, UINT32 dwEventId)	 = 0;
//};
//
//class IFxTimerHandler
//{
//public:
//	virtual ~IFxTimerHandler(){}
//
//	virtual bool			Init(IFxTimer* poTimer) = 0;
//	virtual void			Uninit()	= 0;
//	virtual bool			Run()		= 0;
//	virtual void			Stop()		= 0;
//	virtual bool			AddTimer(int nMicroSecond)	= 0;
//	virtual bool			AddTimer(int nMicroSecond, UINT32 dwEventId)	= 0;
//	virtual bool			DelTimer(int nMicroSecond, UINT32 dwEventId)	= 0;
//	virtual void			Release()	= 0;
//	virtual INT64			GetMicroSecond()	= 0;
//	virtual void			Sleep(int nMicroSecond)	 = 0;
//	virtual void			WakeUp()	= 0;
//};

IFxLock* FxCreateThreadLock();

IFxLock* FxCreateThreadFakeLock();

IFxThreadHandler* FxCreateThreadHandler(IFxThread* poThread, bool bNeedWaitfor);

void FxSleep(UINT32 dwMilliseconds);

char* GetExePath();

//void						FxMakeDaemonProcess();
//
//void						FxGetTimeEx(STimeVal* pstTime);
//
//void						FxSleep(UINT32 dwMilliseconds);
//
//bool						FxCreateDirectory(const char* pszDir);
//
//IFxLogger*					FxCreateLogger();
//
//IFxLogCenterReporter*		FxCreateLogCenterReporter();
//
//IFxTimerHandler*			FxCreateTimer(IFxTimer* poTimer);

#endif
