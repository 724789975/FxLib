#include "test.pb.h"
#include "proto_dispatcher.h"
#include "callback_dispatch.h"

#include "exception_dump.h"

#include <iostream>
#include "fxtimer.h"
#include "fxmeta.h"

#include "share_memory.h"
#include "fix_string.h"

#include "share_mem_container/share_mem_map.h"

#include "property.h"
#include "fix_string.h"

#include "meta/meta.h"
#include "test.h"

#ifdef _WIN32
	__declspec(dllexport) 
#endif	//!_WIN32
BBB::BBB()
	{
		std::cout << "replace function is running!!!!!!!!!\n";
	}
