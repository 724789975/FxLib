#include "sockmgr.h"
#include <time.h>

// [12-06-07] hum add: Max socket count
#define MAX_SOCKET_COUNT UINT32(128)

IMPLEMENT_SINGLETON(FxMySockMgr)

FxMySockMgr::FxMySockMgr()
{
	m_nSockCount		= 0;
	m_nLastCheckTime	= 0;
    m_dwNextId          = 0;
}

FxMySockMgr::~FxMySockMgr()
{

}

bool FxMySockMgr::Init(INT32 nMax)
{
    if (!m_oCPSockPool.Init(nMax, nMax / 2, false, MAX_SOCKET_COUNT))
    {
        return false;
    }

    m_nSockCount    = nMax;
    m_dwNextId      = 0;
	return true;
}

void FxMySockMgr::Uninit()
{
//     CancelIo((HANDLE)hSock);
//     closesocket(hSock);
}

FxConnectSock* FxMySockMgr::Create()
{
	FxConnectSock* poSock = m_oCPSockPool.FetchObj();
    if (NULL == poSock)
    {
        return NULL;
    }

	if (!poSock->Init())
	{
		Release(poSock);
        return NULL;
	}

    poSock->SetState(SSTATE_INVALID);
    poSock->SetSock(INVALID_SOCKET);
    poSock->SetSockId(m_dwNextId++);

    return poSock;
}

FxListenSock* FxMySockMgr::Create(UINT32 dwListenId, IFxSessionFactory* pSessionFactory)
{
	if (m_mapListenSocks.find(dwListenId) != m_mapListenSocks.end())
	{
		return &m_mapListenSocks[dwListenId];
	}

	//����Ŀ�������������~~~~//
//	m_mapListenSocks[dwListenId] = FxListenSock();
	if (!m_mapListenSocks[dwListenId].IFxListenSocket::Init(pSessionFactory))
	{
		m_mapListenSocks.erase(dwListenId);
		return NULL;
	}

	m_mapListenSocks[dwListenId].SetState(SSTATE_INVALID);
	m_mapListenSocks[dwListenId].SetSock(INVALID_SOCKET);
	m_mapListenSocks[dwListenId].SetSockId(m_dwNextId++);

	return &m_mapListenSocks[dwListenId];
}

void FxMySockMgr::Release(FxConnectSock* poSock)
{
	if(NULL == poSock)
    {
		return;
    }
    poSock->Reset();
    m_oCPSockPool.ReleaseObj(poSock);
}

void FxMySockMgr::Release(UINT32 dwListenId)
{
	if (m_mapListenSocks.find(dwListenId) == m_mapListenSocks.end())
	{
		return;
	}

	m_mapListenSocks.erase(dwListenId);
}

