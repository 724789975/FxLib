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
	m_dwInIndex = 0;
	m_dwOutIndex = 0;
	for (int i = 0; i < LOGITEMNUM; ++i)
	{
		m_oLogItems[i].Reset();
	}
}

void LogThread::ThrdFunc()
{
	printf("%s", "thread start !!!!!!!!!!!!!!!!!!!\n");
	while (true)
	{
		{
			if (m_oLogItems[m_dwOutIndex].m_eState == LogItem::LS_None)
			{
				FxSleep(1);
			}
			while (m_oLogItems[m_dwOutIndex].m_eState == LogItem::LS_Writing)
			{
				static int dwCount = 0;
				if (++dwCount > 10)
				{
					dwCount = 0;
					break;
				}
				FxSleep(1);
				if (m_oLogItems[m_dwOutIndex].m_eState != LogItem::LS_Writing)
				{
					break;
				}
			}
			if (m_oLogItems[m_dwOutIndex].m_eState == LogItem::LS_WriteEnd)
			{
				if (m_oLogItems[m_dwOutIndex].m_dwLogType & LT_Screen)
				{
					printf("%s", m_oLogItems[m_dwOutIndex].m_strLog);
				}
				if (m_oLogItems[m_dwOutIndex].m_dwLogType & LT_File)
				{
					FILE* pFile = GetLogFile();
					assert(pFile);
					int ret = fprintf(pFile, "%s", m_oLogItems[m_dwOutIndex].m_strLog);
					if (ret <= 0)
					{
						printf("write to file failed errno : %d\n", ret);
					}
				}
				m_oLogItems[m_dwOutIndex].Reset();
				m_dwOutIndex = (++m_dwOutIndex) % LOGITEMNUM;
			}
		}
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
		LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "FxCreateThreadHandler failed");
		return false;
	}

	return true;
}

bool LogThread::Init()
{
	return Start();
}

void LogThread::BeginLog(unsigned int dwLogType, char* & strLog, unsigned int& dwIndex)
{
	m_pLock->Lock();
	dwIndex = m_dwInIndex;
	if (m_oLogItems[dwIndex].m_eState == LogItem::LS_None)
	{
		m_dwInIndex = (++m_dwInIndex) % LOGITEMNUM;
		m_oLogItems[dwIndex].m_dwLogType = dwLogType;
		m_oLogItems[dwIndex].m_eState = LogItem::LS_Writing;
		memset(m_oLogItems[dwIndex].m_strLog, 0, LOGLENGTH);
		strLog = m_oLogItems[dwIndex].m_strLog;
	}
	else
	{
		strLog = NULL;
	}
	m_pLock->UnLock();
}

void LogThread::EndLog(unsigned int dwIndex)
{
	m_pLock->Lock();
	m_oLogItems[dwIndex].m_eState = LogItem::LS_WriteEnd;
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

LogThread::LogItem::LogItem()
{

}

LogThread::LogItem::~LogItem()
{

}

void LogThread::LogItem::Reset()
{
	m_eState = LS_None;
	m_dwLogType = LT_None;
	memset(m_strLog, 0, LOGLENGTH);
}
