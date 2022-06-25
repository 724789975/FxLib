/***************************************************************************
*   Copyright (C) 2009 by VisualData                                      *
*                                                                         *
*   Redistributed under LGPL license terms.                               *
***************************************************************************/

#include "segvcatch.h"

#include <signal.h>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#define LONG_JMP longjmp
#else
#define LONG_JMP siglongjmp
#endif //!_WIN32


//#include <stdexcept>

using namespace std;

namespace segvcatch
{
	segvcatch::sig_handler handler_sig[128] = {0};

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

	void handle_sig(int sig)
	{
		if (sig & 0xFFFFFF00)
		{
			std::cout << "error sig " << sig << "\n";
			return;
		}
		
		if (handler_sig[sig])
			handler_sig[sig](sig);
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
			handle_sig(SIGSEGV);
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
			handle_sig(SIGFPE);
		}
		catch(std::runtime_error& e)
		{
			segvcatch::long_jmp_env(env, SIGFPE);
		}
	}
#endif

#ifdef HANDLE_SIG

#define SIGNAL_SIG_HANDLER(sig, catch_sig)\
	SIGNAL_HANDLER(catch_sig)\
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

	SIGNAL_SIG_HANDLER(SIGHUP,		catch_hup)
	SIGNAL_SIG_HANDLER(SIGINT,		catch_int)
	SIGNAL_SIG_HANDLER(SIGQUIT,		catch_quit)
	SIGNAL_SIG_HANDLER(SIGILL,		catch_ill)
	SIGNAL_SIG_HANDLER(SIGTRAP,		catch_trap)
	SIGNAL_SIG_HANDLER(SIGABRT,		catch_abrt)
	SIGNAL_SIG_HANDLER(SIGBUS,		catch_bus)
	SIGNAL_SIG_HANDLER(SIGPIPE,		catch_pipe)
	SIGNAL_SIG_HANDLER(SIGALRM,		catch_alrm)
	SIGNAL_SIG_HANDLER(SIGTERM,		catch_term)
	SIGNAL_SIG_HANDLER(SIGCHLD,		catch_chld)
	SIGNAL_SIG_HANDLER(SIGCONT,		catch_cont)
	SIGNAL_SIG_HANDLER(SIGTSTP,		catch_stp)
	SIGNAL_SIG_HANDLER(SIGTTIN,		catch_ttin)
	SIGNAL_SIG_HANDLER(SIGTTOU,		catch_ttout)
	SIGNAL_SIG_HANDLER(SIGURG,		catch_urg)
	SIGNAL_SIG_HANDLER(SIGXCPU,		catch_xcpu)
	SIGNAL_SIG_HANDLER(SIGXFSZ,		catch_xfsz)
	SIGNAL_SIG_HANDLER(SIGVTALRM,	catch_vtalrm)
	SIGNAL_SIG_HANDLER(SIGPROF,		catch_prof)
	SIGNAL_SIG_HANDLER(SIGWINCH,	catch_winch)
	SIGNAL_SIG_HANDLER(SIGIO,		catch_io)
	SIGNAL_SIG_HANDLER(SIGPWR,		catch_pwr)

#endif

#ifdef _WIN32
#include <windows.h>

	static LONG CALLBACK win32_exception_handler(LPEXCEPTION_POINTERS e)
	{
		if (e->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			try
			{
				handle_sig(SIGSEGV);
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
				handle_sig(SIGFPE);
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
	void init_sig(int sig, sig_handler h)
	{
#ifdef _WIN32
		signal(sig, h);
		SetUnhandledExceptionFilter(win32_exception_handler);
#else
		switch (sig)
		{
#ifdef HANDLE_SIG
			case SIGHUP:
			{
				INIT_SIG(SIGHUP,		catch_hup);
			}
			break;
			case SIGINT:
			{
				INIT_SIG(SIGINT,		catch_int);
			}
			break;
			case SIGQUIT:
			{
				INIT_SIG(SIGQUIT,		catch_quit);
			}
			break;
			case SIGILL:
			{
				INIT_SIG(SIGILL,		catch_ill);
			}
			break;
			case SIGTRAP:
			{
				INIT_SIG(SIGTRAP,		catch_trap);
			}
			break;
			case SIGABRT:
			{
				INIT_SIG(SIGABRT,		catch_abrt);
			}
			break;
			case SIGBUS:
			{
				INIT_SIG(SIGBUS,		catch_bus);
			}
			break;
			case SIGPIPE:
			{
				INIT_SIG(SIGPIPE,		catch_pipe);
			}
			break;
#endif	//!HANDLE_SIG
#ifdef HANDLE_SEGV
			case SIGSEGV:
			{
				INIT_SEGV;
				handler_sig[sig] = default_segv;
			}
			break;
#endif	//!HANDLE_SEGV
#ifdef HANDLE_FPE
			case SIGFPE:
			{
				INIT_FPE;
				handler_sig[sig] = default_fpe;
			}
			break;
#endif	//!HANDLE_FPE
#ifdef HANDLE_SIG
			case SIGALRM:
			{
				INIT_SIG(SIGALRM,		catch_alrm);
			}
			break;
			case SIGTERM:
			{
				INIT_SIG(SIGTERM,		catch_term);
			}
			break;
			case SIGCHLD:
			{
				INIT_SIG(SIGCHLD,		catch_chld);
			}
			break;
			case SIGCONT:
			{
				INIT_SIG(SIGCONT,		catch_cont);
			}
			break;
			case SIGTSTP:
			{
				INIT_SIG(SIGTSTP,		catch_stp);
			}
			break;
			case SIGTTIN:
			{
				INIT_SIG(SIGTTIN,		catch_ttin);
			}
			break;
			case SIGTTOU:
			{
				INIT_SIG(SIGTTOU,		catch_ttout);
			}
			break;
			case SIGURG:
			{
				INIT_SIG(SIGURG,		catch_urg);
			}
			break;
			case SIGXCPU:
			{
				INIT_SIG(SIGXCPU,		catch_xcpu);
			}
			break;
			case SIGXFSZ:
			{
				INIT_SIG(SIGXFSZ,		catch_xfsz);
			}
			break;
			case SIGVTALRM:
			{
				INIT_SIG(SIGVTALRM,	catch_vtalrm);
			}
			break;
			case SIGPROF:
			{
				INIT_SIG(SIGPROF,		catch_prof);
			}
			break;
			case SIGWINCH:
			{
				INIT_SIG(SIGWINCH,	catch_winch);
			}
			break;
			case SIGIO:
			{
				INIT_SIG(SIGIO,		catch_io);
			}
			break;
			case SIGPWR:
			{
				INIT_SIG(SIGPWR,		catch_pwr);
			}
			break;
#endif	//!HANDLE_SIG
			default:
				break;
		}

		if (h) handler_sig[sig] = h;
#endif	//!_WIN32
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
