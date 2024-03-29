﻿#include "fxdb.h"
#include "dbmodule.h"

IDBModule* FxDBGetModule()
{
	if (!FxDBModule::Instance())
	{
		if (false == FxDBModule::CreateInstance())
		{
			return NULL;
		}

		if (false == FxDBModule::Instance()->Init())
		{
			LogExe(LogLv_Error, "%s", "Init FxDBModule failed");
			FxDBModule::DestroyInstance();
			return NULL;
		}
	}

	return FxDBModule::Instance();
}
