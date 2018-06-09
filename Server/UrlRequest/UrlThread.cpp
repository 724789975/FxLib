#include "UrlThread.h"
#include "fxcurl.h"
#include "curlmodule.h"


CUrlThread::CUrlThread()
{
	m_bTerminate = false;
	m_poThrdHandler = NULL;
	sprintf(m_szLogPath, "./%s_%s_log.txt", GetExeName(), "curl");
}


CUrlThread::~CUrlThread()
{
}

void CUrlThread::ThrdFunc()
{
	while (!m_bTerminate)
	{
		DoRequest();
		FxSleep(1);
	}

	//退出前确保所有的请求都已执行完
	ClearRequest();
}

void CUrlThread::Stop()
{
	m_bTerminate = true;
	if (m_poThrdHandler != NULL)
	{
		m_poThrdHandler->WaitFor(FX_INFINITE);
		m_poThrdHandler->Release();
		m_poThrdHandler = NULL;
	}
}

bool CUrlThread::Start()
{
	FxCreateThreadHandler(this, true, m_poThrdHandler);
	if (NULL == m_poThrdHandler)
	{
		ThreadLog(LogLv_Error, m_pFile, m_szLogPath, "%s", "FxCreateThreadHandler failed");
		return false;
	}

	return true;
}

bool CUrlThread::AddRequest(IUrlRequest* poRequest)
{
	m_oLock.Lock();
	m_oRequests.push_back(poRequest);
	m_oLock.UnLock();
	return true;
}

void CUrlThread::ClearRequest()
{
	while (DoRequest()) {}
}

bool CUrlThread::DoRequest()
{
	IUrlRequest *poRequest = NULL;
	m_oLock.Lock();
	if (m_oRequests.empty())
	{
		m_oLock.UnLock();
		return false;
	}

	poRequest = m_oRequests.front();
	m_oRequests.pop_front();

	m_oLock.UnLock();

	if (poRequest == NULL)
		return false;

	poRequest->OnRequest();
	FxCurlModule::Instance()->AddResult(poRequest);
	return true;
}
