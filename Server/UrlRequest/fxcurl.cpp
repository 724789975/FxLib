#include "fxcurl.h"
#include "curlmodule.h"

IUrlRequestModule * FxUrlRequestGetModule()
{
	if (!FxCurlModule::Instance())
	{
		if (false == FxCurlModule::CreateInstance())
		{
			return NULL;
		}

		if (false == FxCurlModule::Instance()->Init())
		{
			LogExe(LogLv_Error, "%s", "Init FxDBModule failed");
			FxCurlModule::DestroyInstance();
			return NULL;
		}
	}

	return FxCurlModule::Instance();
}
