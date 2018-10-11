#include "exception_dump.h"
#include "segvcatch.h"
#include "fxtimer.h"
#include "fxmeta.h"

#include <stdexcept>

#ifdef WIN32
#include <windows.h>
#endif // WIN32


namespace Exception_Dump
{
#ifndef WIN32
	void MakeDump()
	{
		static unsigned int g_dwLastExceptionTime = 0;
		if (GetTimeHandler()->GetSecond() - g_dwLastExceptionTime > 1800)
		{
			g_dwLastExceptionTime = GetTimeHandler()->GetSecond();

			char szCmd[256] = { 0 };
			snprintf(szCmd, sizeof(szCmd) - 1, "gcore -o core.%s_%d_%u %u", GetExeName(), g_dwLastExceptionTime, getpid(), getpid());

			system(szCmd);
		}
	}
#endif // !WIN32

	void HandleExceptionSegFault()
	{
#ifdef WIN32
		MessageBox(NULL, "exception : segment fault", "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // WIN32

		throw std::runtime_error("exception : segment fault");
	}

	void HandleExceptionFpError()
	{
#ifdef WIN32
		MessageBox(NULL, "exception : float-point error", "ERROR", MB_ICONINFORMATION | MB_OK);
#else
		MakeDump();
#endif // WIN32
		throw std::runtime_error("exception : float-point error");
	}

	void RegExceptionHandler()
	{
		segvcatch::init_segv(&HandleExceptionSegFault);
		segvcatch::init_fpe(&HandleExceptionFpError);
	}

};


