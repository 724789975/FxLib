#include "curlmodule.h"



FxCurlModule::FxCurlModule()
{
}


FxCurlModule::~FxCurlModule()
{
}

bool FxCurlModule::Init()
{
	curl_global_init(CURL_GLOBAL_ALL);

	return m_oCUrlThread.Start();
}

void FxCurlModule::Close()
{
	curl_global_cleanup();
}

bool FxCurlModule::AddRequest(IUrlRequest *poRequest)
{
	if (NULL == poRequest)
	{
		return false;
	}
	return m_oCUrlThread.AddRequest(poRequest);
}

bool FxCurlModule::Run(UINT32 dwCount /*= -1*/)
{
	bool bRet = false;
	for (UINT32 i = 0; i < dwCount; i++)
	{
		if (!CallBackResult())
		{
			return bRet;
		}
		bRet = true;
	}

	return bRet;
}

void FxCurlModule::AddResult(IUrlRequest* poRequest)
{
	if (NULL == poRequest)
	{
		LogExe(LogLv_Error, "%s", "FxCurlModule::AddResult : NULL Query");
		return;
	}
	m_oLock.Lock();
	m_oResultList.push_back(poRequest);
	m_oLock.UnLock();
}

bool FxCurlModule::CallBackResult()
{
	IUrlRequest *poRequest = NULL;

	m_oLock.Lock();
	if (m_oResultList.empty())
	{
		m_oLock.UnLock();
		return false;
	}

	poRequest = m_oResultList.front();
	m_oResultList.pop_front();
	m_oLock.UnLock();

	if (NULL == poRequest)
	{
		return false;
	}

	poRequest->OnResult();
	poRequest->Release();
	return true;
}

void FxCurlModule::ClearResult()
{
	while (CallBackResult()) {}
}
