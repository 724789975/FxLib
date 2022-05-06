#ifndef __Exception_Dump_H__
#define __Exception_Dump_H__

#include <setjmp.h>
#include <signal.h>


#ifdef _WIN32
#define SET_JMP setjmp
#else
#define SET_JMP sigsetjmp
#endif //!_WIN32

#ifdef _WIN32
	#define __SET_JMP\
	int __jmp_ret = SET_JMP(ExceptionDump::get_jmp_buff());\
	if (0 == __jmp_ret)\
	{\
	}\
	else if (SIGSEGV == __jmp_ret)\
	{\
		std::cout << "SIGSEGV\n";\
	}\
	else if (SIGFPE == __jmp_ret)\
	{\
		std::cout << "SIGFPE\n";\
	}\
	else\
	{\
		std::cout << "error sig : " << __jmp_ret;\
	}
#else
	#define __SET_JMP\
	int __jmp_ret = SET_JMP(ExceptionDump::get_jmp_buff(), 1);\
	if (0 == __jmp_ret)\
	{\
	}\
	else if (SIGSEGV == __jmp_ret)\
	{\
		std::cout << "SIGSEGV\n";\
	}\
	else if (SIGFPE == __jmp_ret)\
	{\
		std::cout << "SIGFPE\n";\
	}\
	else\
	{\
		std::cout << "error sig : " << __jmp_ret;\
	}
#endif  //!_WIN32
	
namespace ExceptionDump
{
	// void HandleExceptionSegFault();
	// void HandleExceptionFpError();
	void RegExceptionHandler();

	jmp_buf& get_jmp_buff();
};

#endif // !__Exception_Dump_H__

