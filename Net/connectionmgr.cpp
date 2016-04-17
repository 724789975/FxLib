#include "connectionmgr.h"

IMPLEMENT_SINGLETON(FxConnectionMgr)

FxConnectionMgr::FxConnectionMgr()
{
	m_dwNextId			= 0;
	m_nMaxConnection	= 0;
}

FxConnectionMgr::~FxConnectionMgr()
{
}

bool FxConnectionMgr::Init(INT32 nMax)
{
	if(!m_oConnPool.Init(nMax, nMax / 2))
	{
		return false;
	}

	m_nMaxConnection = nMax;
	return true;
}

void FxConnectionMgr::Uninit()
{
}

//
// ���̵߳��ã�IOCP�̣߳������̣߳��������������߳����ͬһ�����ͬʱ���õĿ���//
//
FxConnection* FxConnectionMgr::Create()
{
	FxConnection* poConnection = m_oConnPool.FetchObj();
    if (NULL != poConnection)
    {
        poConnection->SetID(++m_dwNextId);
    }

	return poConnection;
}

//
// ���̵߳��ã����̣߳�IOCP�̣߳������̣߳��������������߳����ͬһ�����ͬʱ���õĿ���//
//
void FxConnectionMgr::Release(FxConnection* poConnection)
{
	if(NULL == poConnection)
		return;

	poConnection->Reset();

	m_oConnPool.ReleaseObj(poConnection);
}

