#include "exception_dump.h"
#include "segvcatch.h"

#include <stdexcept>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32


namespace ExceptionDump
{
	char* GetExeName()
	{
		static char strExePath[256] =
		{ 0 };
		static bool bInited = false;

		if (!bInited)
		{
#ifdef _WIN32
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

#ifndef WIN32
	void MakeDump()
	{
		time_t tm = time(NULL);
		static unsigned int g_dwLastExceptionTime = 0;
		if (tm - g_dwLastExceptionTime > 1800)
		{
			g_dwLastExceptionTime = tm;

			char szCmd[256] = { 0 };
			snprintf(szCmd, sizeof(szCmd) - 1, "gcore -o core_%s_%d %u", GetExeName(), g_dwLastExceptionTime, getpid());

			system(szCmd);
		}
	}
#endif // !WIN32

	void HandleSigSegv(int sig)
	{
		signal(sig, HandleSigSegv);
#ifdef _WIN32
		MessageBox(NULL, "exception : segment fault " __FUNCTION__, "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // _WIN32
		segvcatch::long_jmp_env(get_jmp_buff(), SIGSEGV);
	}

	void HandleSigFpe(int sig)
	{
		signal(sig, HandleSigFpe);
#ifdef _WIN32
		MessageBox(NULL, "exception : float-point error " __FUNCTION__, "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // _WIN32
		segvcatch::long_jmp_env(get_jmp_buff(), SIGFPE);
	}

	void HandleExceptionSegFault()
	{
#ifdef _WIN32
		MessageBox(NULL, "exception : segment fault " __FUNCTION__, "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // _WIN32

	
		// segvcatch::long_jmp_env(get_jmp_buff(), SIGSEGV);
		throw std::runtime_error("exception : segment fault");
	}

	void HandleExceptionFpError()
	{
#ifdef _WIN32
		MessageBox(NULL, "exception : float-point error " __FUNCTION__, "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // _WIN32

		// segvcatch::long_jmp_env(get_jmp_buff(), SIGFPE);
		throw std::runtime_error("exception : float-point error");
	}

	void RegExceptionHandler()
	{
		segvcatch::init_segv(&HandleExceptionSegFault);
		segvcatch::init_fpe(&HandleExceptionFpError);

		segvcatch::init_sig(SIGSEGV, &HandleSigSegv);
		segvcatch::init_sig(SIGFPE, &HandleSigFpe);
	}

	jmp_buf& get_jmp_buff()
	{
		return segvcatch::get_jmp_buff();
	}
};


