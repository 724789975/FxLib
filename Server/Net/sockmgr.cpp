#include "sockmgr.h"
#include <time.h>

//IMPLEMENT_SINGLETON(FxMySockMgr)

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
	if (!m_oWebSockPool.Init(nMax, nMax / 2, false, MAX_SOCKET_COUNT))
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

FxTCPConnectSock* FxMySockMgr::CreateCommonTcp()
{
	FxTCPConnectSock* poSock = m_oTCPSockPool.FetchObj();
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

FxWebSocketConnect* FxMySockMgr::CreateWebSocket()
{
	FxWebSocketConnect* poSock = m_oWebSockPool.FetchObj();

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

FxTCPListenSock* FxMySockMgr::CreateCommonTcpListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory)
{
	FxTCPListenSock* pSock = new FxTCPListenSock();
	pSock->IFxListenSocket::Init(pSessionFactory);
	pSock->SetState(SSTATE_INVALID);
	pSock->SetSock(INVALID_SOCKET);
	pSock->SetSockId(m_dwNextId++);
	return pSock;

	static std::map<UINT32, FxTCPListenSock> s_mapListenSocks;
	if (m_mapListenSocks.find(dwPort) != m_mapListenSocks.end())
	{
		return NULL;
	}
	if (!s_mapListenSocks[dwPort].IFxListenSocket::Init(pSessionFactory))
	{
		s_mapListenSocks.erase(dwPort);
		return NULL;
	}
	m_mapListenSocks[dwPort] = &s_mapListenSocks[dwPort];

	s_mapListenSocks[dwPort].SetState(SSTATE_INVALID);
	s_mapListenSocks[dwPort].SetSock(INVALID_SOCKET);
	s_mapListenSocks[dwPort].SetSockId(m_dwNextId++);
	return &s_mapListenSocks[dwPort];
}

FxWebSocketListen* FxMySockMgr::CreateWebSocketListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory)
{
	FxWebSocketListen* pSock = new FxWebSocketListen();
	pSock->IFxListenSocket::Init(pSessionFactory);
	pSock->SetState(SSTATE_INVALID);
	pSock->SetSock(INVALID_SOCKET);
	pSock->SetSockId(m_dwNextId++);
	return pSock;

	static std::map<UINT32, FxWebSocketListen> s_mapListenSocks;
	if (m_mapListenSocks.find(dwPort) != m_mapListenSocks.end())
	{
		return NULL;
	}
	if (!s_mapListenSocks[dwPort].IFxListenSocket::Init(pSessionFactory))
	{
		s_mapListenSocks.erase(dwPort);
		return NULL;
	}
	m_mapListenSocks[dwPort] = &s_mapListenSocks[dwPort];

	s_mapListenSocks[dwPort].SetState(SSTATE_INVALID);
	s_mapListenSocks[dwPort].SetSock(INVALID_SOCKET);
	s_mapListenSocks[dwPort].SetSockId(m_dwNextId++);
	return &s_mapListenSocks[dwPort];
}

void FxMySockMgr::Release(FxTCPConnectSock* poSock)
{
	if(NULL == poSock)
    {
		return;
    }
    poSock->Reset();
    m_oTCPSockPool.ReleaseObj(poSock);
}

void FxMySockMgr::Release(FxWebSocketConnect* poSock)
{
	if (NULL == poSock)
	{
		return;
	}
	poSock->Reset();
	m_oWebSockPool.ReleaseObj(poSock);
}

FxUDPConnectSock * FxMySockMgr::CreateUdpSock()
{
	FxUDPConnectSock* poSock = m_oUDPSockPool.FetchObj();
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

FxUDPListenSock* FxMySockMgr::CreateUdpSockListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory)
{
	FxUDPListenSock* pSock = new FxUDPListenSock();
	pSock->IFxListenSocket::Init(pSessionFactory);
	pSock->SetState(SSTATE_INVALID);
	pSock->SetSock(INVALID_SOCKET);
	pSock->SetSockId(m_dwNextId++);
	return pSock;

	static std::map<UINT32, FxUDPListenSock> s_mapListenSocks;
	if (m_mapListenSocks.find(dwPort) != m_mapListenSocks.end())
	{
		return NULL;
	}
	if (!s_mapListenSocks[dwPort].IFxListenSocket::Init(pSessionFactory))
	{
		s_mapListenSocks.erase(dwPort);
		return NULL;
	}
	m_mapListenSocks[dwPort] = &s_mapListenSocks[dwPort];

	s_mapListenSocks[dwPort].SetState(SSTATE_INVALID);
	s_mapListenSocks[dwPort].SetSock(INVALID_SOCKET);
	s_mapListenSocks[dwPort].SetSockId(m_dwNextId++);
	return &s_mapListenSocks[dwPort];
}

void FxMySockMgr::Release(FxUDPConnectSock* poSock)
{
	if(NULL == poSock)
    {
		return;
    }
    poSock->Reset();
    m_oUDPSockPool.ReleaseObj(poSock);
}

