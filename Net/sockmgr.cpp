#include "sockmgr.h"
#include <time.h>

IMPLEMENT_SINGLETON(FxMySockMgr)

FxMySockMgr::FxMySockMgr()
{
    m_dwNextId          = 0;
}

FxMySockMgr::~FxMySockMgr()
{

}

bool FxMySockMgr::Init(INT32 nMax)
{
    if (!m_oTCPSockPool.Init(nMax, nMax / 2, false, MAX_SOCKET_COUNT))
    {
        return false;
    }
    if (!m_oUDPSockPool.Init(nMax, nMax / 2, false, MAX_SOCKET_COUNT))
    {
        return false;
    }

    m_dwNextId      = 0;
	return true;
}

void FxMySockMgr::Uninit()
{
//     CancelIo((HANDLE)hSock);
//     closesocket(hSock);
}

FxTCPConnectSock* FxMySockMgr::CreateTcpSock()
{
	FxTCPConnectSock* poSock = m_oTCPSockPool.FetchObj();
    if (NULL == poSock)
    {
        return NULL;
    }

	if (!poSock->Init())
	{
		ReleaseTcpSock(poSock);
        return NULL;
	}

    poSock->SetState(SSTATE_INVALID);
    poSock->SetSock(INVALID_SOCKET);
    poSock->SetSockId(m_dwNextId++);

    return poSock;
}

FxTCPListenSock* FxMySockMgr::CreateTcpSock(UINT32 dwListenId, IFxSessionFactory* pSessionFactory)
{
	if (m_mapTcpListenSocks.find(dwListenId) != m_mapTcpListenSocks.end())
	{
		return &m_mapTcpListenSocks[dwListenId];
	}

	//这里的拷贝构造有问题~~~~//
//	m_mapListenSocks[dwListenId] = FxListenSock();
	if (!m_mapTcpListenSocks[dwListenId].IFxListenSocket::Init(pSessionFactory))
	{
		m_mapTcpListenSocks.erase(dwListenId);
		return NULL;
	}

	m_mapTcpListenSocks[dwListenId].SetState(SSTATE_INVALID);
	m_mapTcpListenSocks[dwListenId].SetSock(INVALID_SOCKET);
	m_mapTcpListenSocks[dwListenId].SetSockId(m_dwNextId++);

	return &m_mapTcpListenSocks[dwListenId];
}

void FxMySockMgr::ReleaseTcpSock(FxTCPConnectSock* poSock)
{
	if(NULL == poSock)
    {
		return;
    }
    poSock->Reset();
    m_oTCPSockPool.ReleaseObj(poSock);
}

void FxMySockMgr::ReleaseTcpSock(UINT32 dwListenId)
{
	if (m_mapTcpListenSocks.find(dwListenId) == m_mapTcpListenSocks.end())
	{
		return;
	}

	m_mapTcpListenSocks.erase(dwListenId);
}

