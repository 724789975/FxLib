#include "log_thread.h"
#include "fxmeta.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <string.h>

LogThread::LogThread()
{
	m_pLock = FxCreateThreadLock();
}

void LogThread::ThrdFunc()
{
	printf("%s", "thread start !!!!!!!!!!!!!!!!!!!\n");
	while (true)
	{
		if (m_bPrint)
		{
			bool bEmpty = false;
			unsigned int dwIndex = m_dwCurrentIndex ? 0 : 1;
			if (strlen(m_strScreenLog[dwIndex]) > 0)
			{
				printf(m_strScreenLog[dwIndex]);
			}
			else
			{
				bEmpty = true;
			}

			if (strlen(m_strFileLog[dwIndex]) > 0)
			{
				FILE* pFile = GetLogFile();
				assert(pFile);
				int ret = fprintf(pFile, m_strFileLog[dwIndex]);
				if (ret <= 0)
				{
					printf("write to file failed errno : %d\n", ret);
				}
				bEmpty = false;
			}

			if (bEmpty)
			{
				printf("write empty~~~~~~~~~~~~~~~~~~!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}

			m_bPrint = false;
			memset(m_strScreenLog[dwIndex], 0, LOGLENGTH);
			memset(m_strFileLog[dwIndex], 0, LOGLENGTH);
		}
		FxSleep(1);
	}
	printf("%s", "thread end!!!!!!!!!!!!!!!!!!!\n");
}

void LogThread::Stop()
{
	if (m_poThrdHandler != NULL)
	{
		m_poThrdHandler->WaitFor(0xffffffff);
		m_poThrdHandler->Release();
		m_poThrdHandler = NULL;
	}
}

bool LogThread::Start()
{
	m_poThrdHandler = FxCreateThreadHandler(this, true);
	if (NULL == m_poThrdHandler)
	{
		printf("%s", "FxCreateThreadHandler failed");
		return false;
	}

	return true;
}

bool LogThread::Init()
{
	m_dwInIndex = 0;
	m_dwOutIndex = 0;
	memset(m_strScreenLog, 0, 2 * LOGLENGTH);
	memset(m_strFileLog, 0, 2 * LOGLENGTH);
	m_dwCurrentIndex = 0;
	m_bPrint = false;

	return Start();
}

static unsigned int dwScreenLogIndex = 0;
static unsigned int dwFileLogIndex = 0;
void LogThread::ReadLog(unsigned int dwLogType, char* strLog)
{
	m_pLock->Lock();
	bool bPrint = false;
	if (dwScreenLogIndex + 2048 > LOGLENGTH)
	{
		bPrint = true;
	}
	if (dwFileLogIndex + 2048 > LOGLENGTH)
	{
		bPrint = true;
	}

	if (bPrint)
	{
		unsigned int dwIndex = m_dwCurrentIndex ? 0 : 1;
		while (m_strScreenLog[dwIndex][0] | m_strFileLog[dwIndex][0])
		{
			FxSleep(1);
		}
		dwScreenLogIndex = 0;
		dwFileLogIndex = 0;
		m_dwCurrentIndex = dwIndex;
		m_bPrint = bPrint;
	}
	if (dwLogType & LT_Screen)
	{
		char* pStr = (char*)(m_strScreenLog[m_dwCurrentIndex]) + dwScreenLogIndex;
		dwScreenLogIndex += sprintf(pStr, strLog);
	}
	if (dwLogType & LT_File)
	{
		char* pStr = (char*)(m_strFileLog[m_dwCurrentIndex]) + dwFileLogIndex;
		dwFileLogIndex += sprintf(pStr, strLog);
	}
	m_pLock->UnLock();
}

FILE* LogThread::GetLogFile()
{
	char strLogPath[512] = { 0 };
#ifdef WIN32
	sprintf(strLogPath, "%s%s%s%s", GetExePath(), "\\", GetExeName(), "_log.txt");
#else
	sprintf(strLogPath, "%s%s%s%s", GetExePath(), "/", GetExeName(), "_log.txt");
#endif // WIN32

	static char sstrPath[512] =
	{ 0 };
	static FILE* pFile = NULL;
	if (strcmp(strLogPath, sstrPath) != 0)
	{
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}
		sprintf(sstrPath, "%s", strLogPath);
		pFile = fopen(sstrPath, "a+");
	}
	return pFile;
}

