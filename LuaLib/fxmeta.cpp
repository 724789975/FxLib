#include "fxmeta.h"
#include <set>
#include <list>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#include <process.h>

#include <psapi.h>
#include <cstddef>  
#include <dbghelp.h>
#else
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <netinet/in.h>
#include <execinfo.h>
#endif // WIN32

#ifndef INFINITE
#define INFINITE			 0xffffffff
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#define EPOCHFILETIME (116444736000000000i64)

class FxCriticalLock: public IFxLock
{
	// Can't be copied
	void operator =(FxCriticalLock &locker)
	{
	}
public:
#ifdef WIN32
	FxCriticalLock()
	{
		InitializeCriticalSection(&m_lock);
	}
	~FxCriticalLock()
	{
		DeleteCriticalSection(&m_lock);
	}
	void Lock()
	{
		EnterCriticalSection(&m_lock);
	}
	void UnLock()
	{
		LeaveCriticalSection(&m_lock);
	}
#else
	FxCriticalLock()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~FxCriticalLock()
	{
		pthread_mutex_destroy(&m_lock);
	}
	void Lock()
	{
		pthread_mutex_lock(&m_lock);
	}
	void UnLock()
	{
		pthread_mutex_unlock(&m_lock);
	}
#endif

	virtual void Release()
	{
		if (NULL != this)
			delete this;
	}

private:
#ifdef WIN32
	CRITICAL_SECTION m_lock;
#else
	pthread_mutex_t m_lock;
#endif
};

class FxFakeLock: public IFxLock
{
	// Can't be copied
	void operator =(FxFakeLock &locker)
	{
	}
public:
	FxFakeLock()
	{
	}
	~FxFakeLock()
	{
	}
	void Lock()
	{
	}
	void UnLock()
	{
	}
	virtual void Release()
	{
		if (NULL != this)
			delete this;
	}
};

IFxLock*
FxCreateThreadLock()
{
	return new FxCriticalLock;
}

IFxLock*
FxCreateThreadFakeLock()
{
	return new FxFakeLock;
}

void FxSleep(UINT32 dwMilliseconds)
{
#ifdef WIN32
	Sleep(dwMilliseconds);
#else
	usleep(dwMilliseconds * 1000);
#endif // WIN32

}

FILE* GetLogFile(const char* strPath)
{
	static char sstrPath[512] =
	{ 0 };
	static FILE* pFile = NULL;
	if (strcmp(strPath, sstrPath) != 0)
	{
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}
		sprintf(sstrPath, "%s", strPath);
		pFile = fopen(sstrPath, "a+");
	}
	return pFile;
}

bool Log(char* strBuffer, unsigned int dwLen, const char* strFmt, ...)
{
	va_list arg;
	va_start(arg, strFmt);
	int nCheck = vsnprintf(strBuffer, dwLen, strFmt, arg);
	va_end(arg);
	if (nCheck < 0)
	{
		return false;
	}
	if (nCheck >= (int) dwLen)
	{
		return false;
	}
	sprintf(strBuffer + nCheck, "%s", "\n");
	return true;
}

char* PrintTrace()
{
	static char strTrace[1024];
	memset(strTrace, 0, 1024);
	int nLen = 0;
	nLen += sprintf(strTrace + nLen, "%s\n", " [Trace] ___________begin___________");
#ifdef WIN32
	unsigned int   i;
	void         * stack[100];
	unsigned short frames;
	SYMBOL_INFO  * symbol = NULL;
	HANDLE         process;

	process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	frames = CaptureStackBackTrace(0, 100, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	if (symbol)
	{
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for (i = 0; i < frames; i++)
		{
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

			nLen += sprintf(strTrace + nLen, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, (unsigned int)(symbol->Address));
		}
		free(symbol);
	}

#else
	void *bt[20];
	char **strings;
	int sz;

	sz = backtrace(bt, 20);
	strings = backtrace_symbols(bt, sz);
	for(int i = 0; i < sz; ++i)
	{
		nLen += sprintf(strTrace + nLen, "%s\n", strings[i]);
	}
#endif // WIN32
	nLen += sprintf(strTrace + nLen, "%s\n", " [Trace] ___________end___________");

	return strTrace;
}

void ListDir(const char* pDirName, ListDirAndLoadFile* pListDirAndLoadFile)
{
#ifdef WIN32
	char szFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];
	char strFileName[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	strcpy(szFind, pDirName);

	strcat(szFind, "*.*");
	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind) return;
	while (1)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				strcpy(szFile, pDirName);
				//strcat(szFile,"//");  
				strcat(szFile, FindFileData.cFileName);
				strcat(szFile, "//");
				ListDir(szFile, pListDirAndLoadFile);
			}
		}
		else
		{

			strcpy(strFileName, pDirName);
			strcat(strFileName, FindFileData.cFileName);
			pListDirAndLoadFile->LoadFile(strFileName);
		}


		if (!FindNextFile(hFind, &FindFileData)) break;
	}
	FindClose(hFind);
#else
	DIR* dp;
	struct dirent* pDirp;

	/* open dirent directory */
	if ((dp = opendir(pDirName)) == NULL)
	{
		perror(pDirName);
		return;
	}

	/**
	 * read all files in this dir
	 **/
	while ((pDirp = readdir(dp)) != NULL)
	{
		if (strcmp(pDirp->d_name, ".") == 0 || strcmp(pDirp->d_name, "..") == 0)
		{
			continue;
		}
		switch (pDirp->d_type)
		{
			case DT_REG:
			{
				std::string strPath(pDirName);
				std::string strFileName(pDirp->d_name);
				std::string strFullFilePath = strPath + "/" + strFileName;
				pListDirAndLoadFile->LoadFile(strFullFilePath.c_str());
			}
			break;
			case DT_DIR:
			{
				//directory
				std::string strPath(pDirName);
				std::string strDirName(pDirp->d_name);
				std::string strFullDirPath = strPath + "/" + strDirName;
				ListDir(strFullDirPath.c_str(), pListDirAndLoadFile);
			}
			break;
			default:
			{
				return;
			}
			break;
		}
	}
#endif
}

char* GetExePath()
{
	static char strWorkPath[256] =
	{ 0 };
	static bool bInited = false;

	if (!bInited)
	{
#ifdef WIN32
		//GetCurrentDirectory(256, strWorkPath);

		GetModuleFileName(NULL, strWorkPath, 256);
		//??????   
		for (int i = strlen(strWorkPath); i >= 0; i--)
		{
			if (strWorkPath[i] == '\\')
			{
				strWorkPath[i] = '\0';
				break;
			}
		}
#else
		char strSysfile[256] =
		{ 0 };
		sprintf(strSysfile, "/proc/%d/exe", getpid());

		int nRet = readlink(strSysfile, strWorkPath, 256);
		if ((nRet > 0) & (nRet < 256))
		{
			bInited = true;

			strWorkPath[nRet] = 0;

			for (int i = nRet; i >= 0; --i)
			{
				if (strWorkPath[i] == '/')
				{
					strWorkPath[i] = '\0';
					break;
				}
				strWorkPath[i] = '\0';
			}
		}
#endif
	}

	return strWorkPath;
}

char* GetExeName()
{
	static char strExePath[256] =
	{ 0 };
	static bool bInited = false;

	if (!bInited)
	{
#ifdef WIN32
		//GetCurrentDirectory(256, strWorkPath);

		GetModuleFileName(NULL, strExePath, 256);
		//??????   
		for (int i = strlen(strExePath); i >= 0; i--)
		{
			if (strExePath[i] == '\\')
			{
				memcpy(strExePath, &(strExePath[i + 1]), 256 - (i + 1) - 1);
				//strExePath[i] = '\0';
				break;
			}
		}
#else
		char strSysfile[256] =
		{ 0 };
		sprintf(strSysfile, "/proc/%d/exe", getpid());

		int nRet = readlink(strSysfile, strExePath, 256);
		if ((nRet > 0) & (nRet < 256))
		{
			bInited = true;

			strExePath[nRet] = 0;

			for (int i = nRet; i >= 0; --i)
			{
				if (strExePath[i] == '/')
				{
					memcpy(strExePath, &(strExePath[i + 1]), 256 - (i + 1) - 1);
					//strExePath[i] = '\0';
					break;
				}
				//strExePath[i] = '\0';
			}
		}
#endif
	}

	return strExePath;
}

