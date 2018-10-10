#ifndef __Exception_Dump_H__
#define __Exception_Dump_H__

namespace Exception_Dump
{
	static void HandleExceptionSegFault();
	static void HandleExceptionFpError();
	static void RegExceptionHandler();
};

#endif // !__Exception_Dump_H__

