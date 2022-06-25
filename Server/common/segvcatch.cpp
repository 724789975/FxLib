/***************************************************************************
*   Copyright (C) 2009 by VisualData                                      *
*                                                                         *
*   Redistributed under LGPL license terms.                               *
***************************************************************************/

#include "segvcatch.h"

#include <signal.h>
#include <stdexcept>

#ifdef _WIN32
#define LONG_JMP longjmp
#else
#define LONG_JMP siglongjmp
#endif //!_WIN32


//#include <stdexcept>

using namespace std;

namespace segvcatch
{
	segvcatch::sig_handler handler_segv = 0;
	segvcatch::sig_handler handler_fpe = 0;
	segvcatch::sig_handler handler_sig = 0;

	jmp_buf env;

#if defined __GNUC__ && __linux

#ifdef __i386__
#include "i386-signal.h"
#endif /*__i386__*/

#ifdef __x86_64__
#include "x86_64-signal.h"
#endif /*__x86_64__*/

#endif /*defined __GNUC__ && __linux*/

	void default_segv(int sig)
	{
		throw "Segmentation fault";
	}

	void default_fpe(int sig)
	{
		throw "Floating-point exception";
	}


	void handle_segv(int sig)
	{
		if (handler_segv)
			handler_segv(sig);
	}

	void handle_fpe(int sig)
	{
		if (handler_fpe)
			handler_fpe(sig);
	}

	void handle_sig(int sig)
	{
		if (handler_sig)
			handler_sig(sig);
	}

#if defined (HANDLE_SEGV) || defined(HANDLE_FPE)

#include <execinfo.h>
	/* Unblock a signal.  Unless we do this, the signal may only be sent
	once.  */

	static void unblock_signal(int signum __attribute__((__unused__)))
	{
#ifdef _POSIX_VERSION
		sigset_t sigs;
		sigemptyset(&sigs);
		sigaddset(&sigs, signum);
		sigprocmask(SIG_UNBLOCK, &sigs, NULL);
#endif
	}
#endif

#ifdef HANDLE_SEGV

	SIGNAL_HANDLER(catch_segv)
	{
		unblock_signal(SIGSEGV);
		MAKE_THROW_FRAME(nullp);
		try
		{
			handle_segv(SIGSEGV);
		}
		catch(std::runtime_error& e)
		{
			segvcatch::long_jmp_env(env, SIGSEGV);
		}
	}
#endif

#ifdef HANDLE_FPE

	SIGNAL_HANDLER(catch_fpe)
	{
		unblock_signal(SIGFPE);
#ifdef HANDLE_DIVIDE_OVERFLOW
		HANDLE_DIVIDE_OVERFLOW;
#else
		MAKE_THROW_FRAME(arithexception);
#endif
		try
		{
			handle_fpe(SIGFPE);
		}
		catch(std::runtime_error& e)
		{
			segvcatch::long_jmp_env(env, SIGFPE);
		}
	}
#endif

#ifdef HANDLE_SIG

#define SIGNAL_SIG_HANDLER(sig, catch_sig)\
	SIGNAL_HANDLER(##catch_sig)\
	{\
		unblock_signal(sig);\
		MAKE_THROW_FRAME(nullp);\
		try\
		{\
			handle_sig(sig);\
		}\
		catch(std::runtime_error& e)\
		{\
			segvcatch::long_jmp_env(env, sig);\
		}\
	}

	SIGNAL_SIG_HANDLER(SIGINT, catch_int)
	SIGNAL_SIG_HANDLER(SIGABRT, catch_abrt)

#endif

#ifdef _WIN32
#include <windows.h>

	static LONG CALLBACK win32_exception_handler(LPEXCEPTION_POINTERS e)
	{
		if (e->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			try
			{
				handle_segv(SIGSEGV);
			}
			catch(std::runtime_error& e)
			{
				segvcatch::long_jmp_env(env, SIGSEGV);
			}
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else if (e->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO)
		{
			try
			{
				handle_fpe(SIGFPE);
			}
			catch(std::runtime_error& e)
			{
				segvcatch::long_jmp_env(env, SIGFPE);
			}
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else
			return EXCEPTION_CONTINUE_SEARCH;
	}
#endif
}

namespace segvcatch
{

	void init_segv(sig_handler h)
	{
		if (h)
			handler_segv = h;
		else
			handler_segv = default_segv;
#ifdef HANDLE_SEGV
		INIT_SEGV;
#else
		signal(SIGSEGV, h);
#endif

#ifdef _WIN32
		SetUnhandledExceptionFilter(win32_exception_handler);
#endif
	}

	void init_fpe(sig_handler h)
	{
		if (h)
			handler_fpe = h;
		else
			handler_fpe = default_fpe;
#ifdef HANDLE_FPE
		INIT_FPE;
#else
		signal(SIGFPE, h);
#endif

#ifdef _WIN32
		SetUnhandledExceptionFilter(win32_exception_handler);
#endif
	}

	void init_sig(int sig, sig_handler h)
	{
#ifdef _WIN32
		signal(sig, h);
#else
#endif
	}

	jmp_buf& get_jmp_buff()
	{
		return env;
	}

	void long_jmp_env(jmp_buf& env, int sig)
	{
		LONG_JMP(env, sig);
	}

}
