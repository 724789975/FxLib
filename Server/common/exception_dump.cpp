#include "exception_dump.h"
#include "segvcatch.h"

#include <stdexcept>

namespace Exception_Dump
{
	void HandleExceptionSegFault()
	{
#ifdef WIN32
		throw std::runtime_error("exception : segment fault");
#else
#endif // WIN32

	}

	void HandleExceptionFpError()
	{
#ifdef WIN32
		throw std::runtime_error("exception : float-point error");
#else
#endif // WIN32
	}

	void RegExceptionHandler()
	{
		segvcatch::init_segv(&HandleExceptionSegFault);
		segvcatch::init_fpe(&HandleExceptionFpError);
	}

};


