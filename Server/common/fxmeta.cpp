#include "fxmeta.h"
#include <set>
#include <list>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include "defines.h"

#ifdef WIN32
#include <Windows.h>
#include <process.h>

#include <psapi.h>
#include <cstddef>
#include <dbghelp.h>
#else
#include <pthread.h>
#include <signal.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <netinet/in.h>
#include <execinfo.h>
#include <sys/stat.h>
#endif // WIN32

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

int GetPid()
{
#ifdef WIN32
	return _getpid();
#else
	return (int)getpid();
#endif // WIN32
}

FILE* GetLogFile()
{
	static FILE* pFile = NULL;
	static char sstrPath[512] = { 0 };
	static char strLogPath[512] = { 0 };
	unsigned int dwTime = GetTimeHandler()->GetSecond() - GetTimeHandler()->GetSecond() % 3600;
	string_snprintf(strLogPath, 512 - 1, "./%s_%d_%d_exe_log.txt", GetExeName(), dwTime, GetPid());

	//if (strcmp(strLogPath, sstrPath) != 0)
	//{
	//	if (pFile)
	//	{
	//		fclose(pFile);
	//		pFile = NULL;
	//	}
	//	sprintf(sstrPath, "%s", strLogPath);
	//	pFile = fopen(sstrPath, "a+");
	//}
	if (Access(strLogPath, 0) == -1)
	{
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}
		pFile = fopen(strLogPath, "a+");
		setvbuf(pFile, (char *)NULL, _IOLBF, 1024);
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
	string_sprintf(strBuffer + nCheck, 2, "%s", "\n");
	return true;
}

#define TRACE_SIZE 100
void PrintTrace(char* strTrace, int dwLen)
{
	int nLen = 0;
	nLen += string_sprintf(strTrace + nLen, dwLen - 1, "%s\n", " [Trace] ___________begin___________");
#ifdef WIN32
	unsigned int   i;
	void         * stack[TRACE_SIZE];
	unsigned short frames;
	SYMBOL_INFO  * symbol = NULL;
	HANDLE         process;

	process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	frames = CaptureStackBackTrace(0, TRACE_SIZE, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	if (symbol)
	{
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for (i = 1; i < frames; i++)
		{
			SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

			nLen += string_sprintf(strTrace + nLen, dwLen - nLen - 1, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, (unsigned int)(symbol->Address));
		}
		free(symbol);
	}

#else
	void *bt[TRACE_SIZE];
	int sz = backtrace(bt, TRACE_SIZE);
	char **strings = backtrace_symbols(bt, sz);
	for(int i = 1; i < sz; ++i)
	{
		nLen += string_sprintf(strTrace + nLen, dwLen - nLen - 1, "%s\n", strings[i]);
	}
	free(strings);
	strings = NULL;
#endif // WIN32
	nLen += string_sprintf(strTrace + nLen, dwLen - nLen - 1, "%s\n", " [Trace] ___________end___________");
}

void ListDir(const char* pDirName, ListDirAndLoadFile* pListDirAndLoadFile)
{
#ifdef WIN32
	char szFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];
	char strFileName[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	string_cpy_s(szFind, MAX_PATH, pDirName);

	string_cat_s(szFind, MAX_PATH - 1, "*.*");
	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind) return;
	while (1)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				string_cpy_s(szFile, MAX_PATH, pDirName);
				//strcat(szFile,"//");  
				string_cat_s(szFile, MAX_PATH, FindFileData.cFileName);
				string_cat_s(szFile, MAX_PATH, "//");
				ListDir(szFile, pListDirAndLoadFile);
			}
		}
		else
		{

			string_cpy_s(strFileName, MAX_PATH, pDirName);
			string_cat_s(strFileName, MAX_PATH, FindFileData.cFileName);
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
#if 1
		//有的系统不能用readdir 要用stat
		struct stat buf;
		std::string strPath(pDirName);
		std::string strFileName(pDirp->d_name);
		std::string strFullFilePath = strPath + "/" + strFileName;
		stat(strFullFilePath.c_str(), &buf);
		if (buf.st_mode & S_IFREG)
		{
			std::string strPath(pDirName);
			std::string strFileName(pDirp->d_name);
			std::string strFullFilePath = strPath + "/" + strFileName;
			pListDirAndLoadFile->LoadFile(strFullFilePath.c_str());
		}
		else if (buf.st_mode & S_IFDIR)
		{
			//directory
			std::string strPath(pDirName);
			std::string strDirName(pDirp->d_name);
			std::string strFullDirPath = strPath + "/" + strDirName;
			ListDir(strFullDirPath.c_str(), pListDirAndLoadFile);
		}
		else
		{
			continue;
		}
#else
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
#endif
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
		string_sprintf(strSysfile, 256 -1, "/proc/%d/exe", getpid());

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
		for (int i = strlen(strExePath); i >= 0; i--)
		{
			if (strExePath[i] == '.')
			{
				strExePath[i] = 0;
			}
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
		string_sprintf(strSysfile, 256 - 1, "/proc/%d/exe", getpid());

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

