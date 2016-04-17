#include "fxdb.h"
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
			LogScreen("Init FxDBModule failed");
			FxDBModule::DestroyInstance();
			return NULL;
		}
	}

	return FxDBModule::Instance();
}
