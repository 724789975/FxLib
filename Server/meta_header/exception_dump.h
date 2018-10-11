#ifndef __Exception_Dump_H__
#define __Exception_Dump_H__

namespace ExceptionDump
{
	void HandleExceptionSegFault();
	void HandleExceptionFpError();
	void RegExceptionHandler();
};

#endif // !__Exception_Dump_H__

