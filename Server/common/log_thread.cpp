#include "log_thread.h"
#include "fxmeta.h"
#include "defines.h"
#include "fxtimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

LogItem::LogItem()
	: m_dwLogLength(0)
{
	memset(m_szScreenLog, 0, LOGLENGTH);
}

void LogItem::Reset()
{
	memset(m_szScreenLog, 0, LOGLENGTH);
	m_dwLogLength = 0;

	m_streamLogStream.clear();
	m_streamLogStream.str("");
}

std::stringstream& LogItem::GetStream()
{
	return m_streamLogStream;
}

LogThread::LogThread(bool bPrintScene)
	: m_bPrintScene(bPrintScene)
{
	m_dwInIndex = 0;
	m_dwOutIndex = 0;
	memset(m_strScreenLog, 0, 2 * LOGLENGTH);
	memset(m_strFileLog, 0, 2 * LOGLENGTH);
	m_dwCurrentIndex = 0;
	m_bPrint = false;
	m_bStop = false;
	Assert(GetLogFile());
}

void LogThread::ThrdFunc()
{
	LogExe(LogLv_Info, "thread : %d start!!!!!!!!!!!!!!!!!!!", m_poThrdHandler->GetThreadId());
	while (!m_bStop)
	{
		if (m_bPrint)
		{
			unsigned int dwIndex = m_dwCurrentIndex + 1;
			dwIndex %= 2;

			LogItem& refItem = m_oLogItem[dwIndex];
			std::stringstream& refStream = refItem.GetStream();
			if (refStream.str().size())
			{
			}
		}
		FxSleep(1);
	}

}

void LogThread::Stop()
{
	m_bStop = true;
	if (m_poThrdHandler != NULL)
	{
		m_poThrdHandler->WaitFor(0xffffffff);
		m_poThrdHandler->Release();
		m_poThrdHandler = NULL;
	}
}

bool LogThread::Start()
{
	FxCreateThreadHandler(this, true, m_poThrdHandler);
	if (NULL == m_poThrdHandler)
	{
		printf("%s", "FxCreateThreadHandler failed");
		return false;
	}

	return true;
}

bool LogThread::Init()
{
	return Start();
}

void LogThread::ReadLog(unsigned int dwLogType, char* strLog)
{
	if (m_bPrintScene)
	{
		printf("%s", strLog);
	}

	//FxLockImp(this->m_pLock);
}

#ifdef _WIN32
#define Access _access
#else
#define Access access
#endif

FILE* LogThread::GetLogFile()
{
	static FILE* pFile = NULL;
	static bool bInted = false;
	static char sstrPath[512] = { 0 };
	static char strLogPath[512] = { 0 };
	if (bInted)
	{
		if (Access(strLogPath, 0) == -1)
		{
			fclose(pFile);
			pFile = fopen(sstrPath, "a+");
		}
		return pFile;
	}

	if (m_bPrintScene)
	{
		//主线程 根据时间更换日志
#ifdef _WIN32
		string_sprintf(strLogPath, 0, "%s%s%s%s", GetExePath(), "\\", GetExeName(), "_log.txt");
#else
		string_sprintf(strLogPath, 0, "%s%s%s%s", GetExePath(), "/", GetExeName(), "_log.txt");
#endif // _WIN32
	}
	else
	{
#ifdef _WIN32
		string_sprintf(strLogPath, 0, "%s%s%s%s", GetExePath(), "\\", GetExeName(), "_log.txt");
#else
		string_sprintf(strLogPath, 0, "%s%s%s%s", GetExePath(), "/", GetExeName(), "_log.txt");
#endif // _WIN32
	}

	if (strcmp(strLogPath, sstrPath) != 0)
	{
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}
		string_sprintf(sstrPath, 0, "%s", strLogPath);
		pFile = fopen(sstrPath, "a+");
		if (pFile)
		{
			bInted = true;
		}
	}
	return pFile;
}

LogImp::LogImp(LogThread& refLog)
	: m_refLog(refLog)
{

}

LogImp::~LogImp()
{
	m_refLog.m_dwCurrentIndex += 1;
	m_refLog.m_dwCurrentIndex %= 2;
}
