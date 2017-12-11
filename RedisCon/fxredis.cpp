#include "fxredis.h"
#include "dbmodule.h"

IRedisModule* FxRedisGetModule()
{
	if (!FxRedisModule::Instance())
	{
		if (false == FxRedisModule::CreateInstance())
		{
			return NULL;
		}

		if (false == FxRedisModule::Instance()->Init())
		{
			LogExe(LogLv_Error, "%s", "Init FxDBModule failed");
			FxRedisModule::DestroyInstance();
			return NULL;
		}
	}

	return FxRedisModule::Instance();
}
