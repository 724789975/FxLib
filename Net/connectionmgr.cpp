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
// 多线程调用：IOCP线程，连接线程，但不存在两个线程针对同一对象口同时调用的可能//
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
// 多线程调用：主线程，IOCP线程，连接线程，但不存在两个线程针对同一对象口同时调用的可能//
//
void FxConnectionMgr::Release(FxConnection* poConnection)
{
	if(NULL == poConnection)
		return;

	poConnection->Reset();

	m_oConnPool.ReleaseObj(poConnection);
}

