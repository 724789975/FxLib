#include "net.h"
#include "sockmgr.h"
#include "connectionmgr.h"
#include "loopbuff.h"
#include "iothread.h"

IMPLEMENT_SINGLETON(FxNetModule);
FxNetModule::FxNetModule()
{
    m_nNetThreadCount = 0;
    m_nMaxConnectionCount = MAX_CONNECTION_COUNT;
    m_nTotalEventCount = MAX_NETEVENT_PERSOCK * MAX_CONNECTION_COUNT;
    m_bInit = false;
}

FxNetModule::~FxNetModule()
{

}

bool FxNetModule::Init()
{
    if (!__CreateComponent())
    {
        return false;
    }

    if (!__InitComponent())
    {
        __DestroyComponent();
        return false;
    }

    m_bInit = true;
    return true;
}

bool FxNetModule::__CreateComponent()
{
    if (!FxConnectionMgr::CreateInstance())
    {
        return false;
    }

    if (!FxMySockMgr::CreateInstance())
    {
        return false;
    }

    if (!FxLoopBuffMgr::CreateInstance())
    {
        return false;
    }

    return true;
}

bool FxNetModule::__InitComponent()
{
    if (!FxConnectionMgr::Instance()->Init(m_nMaxConnectionCount))
    {
        return false;
    }

    if (!FxMySockMgr::Instance()->Init(m_nMaxConnectionCount))
    {
        return false;
    }

    if (!FxLoopBuffMgr::Instance()->Init(m_nMaxConnectionCount))
    {
        return false;
    }

    if (0 >= m_nNetThreadCount)
    {
        m_nNetThreadCount = LINUX_NETTHREAD_COUNT;
    }

	m_pEpollHandlers = new FxIoThread[m_nNetThreadCount];
	if (NULL == m_pEpollHandlers)
	{
	   	return false;
	}

    for (int i = 0; i < m_nNetThreadCount; i++)
    {
		FxIoThread* poEpollHandler = m_pEpollHandlers + i;
      	poEpollHandler->Init(m_nMaxConnectionCount);
    }

	m_pEventQueue = new TEventQueue<IFxSocket*>;
	m_pEventQueue->Init(m_nTotalEventCount);

    return true;
}

void FxNetModule::__UninitComponent()
{
	// todo
    for (int i = 0; i < m_nNetThreadCount; i++)
    {
        (m_pEpollHandlers + i)->Uninit();
    }
}

void FxNetModule::__DestroyComponent()
{
    FxConnectionMgr::DestroyInstance();
}

void FxNetModule::Uninit()
{
    __UninitComponent();
    __DestroyComponent();
}

IFxListenSocket* FxNetModule::Listen(IFxSessionFactory* pSessionFactory, UINT32 dwListenId, UINT32 dwIP, UINT16 dwPort)
{
	IFxListenSocket* pListenSocket = FxMySockMgr::Instance()->Create(dwListenId, pSessionFactory);
	if (pListenSocket == NULL)
	{
		return NULL;
	}
	if (pListenSocket->Listen(dwIP, dwPort))
	{
		return pListenSocket;
	}
	return NULL;
}

SOCKET FxNetModule::Connect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect /*= false*/)
{
	//FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
	//if (NULL == poSock)
	//{
	//	return INVALID_SOCKET;
	//}

	FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
	if (NULL == poConnection)
	{
		//FxMySockMgr::Instance()->Release(poSock);
		return INVALID_SOCKET;
	}

	poConnection->SetRemoteIP(dwIP);
	poConnection->SetRemotePort(wPort);
	poConnection->SetReconnect(bReconnect);
	poConnection->SetSession(poSession);
	//poConnection->SetSock(poSock);
	//poConnection->SetID(poSock->GetSockId());
	poSession->Init(poConnection);

	//poSock->SetConnection(poConnection);

	//return poSock->Connect();
	return poConnection->Reconnect();
}

bool FxNetModule::PushNetEvent(IFxSocket* poSock)
{
	if(m_pEventQueue->PushBack(poSock))
	{
		return true;
	}

	return false;
}

bool FxNetModule::Run(UINT32 dwCount)
{
    bool bRet = true;

	IFxSocket** ppSock = NULL;
    for (UINT32 i = 0; i < dwCount; i++)
    {
        ppSock = m_pEventQueue->PopFront();
        if (NULL == ppSock)
        {
            bRet = false;
			break;
        }

		IFxSocket* poSock = *ppSock;
        if (NULL == poSock)
        {
            bRet = false;
			break;
        }

        poSock->ProcEvent();
    }

	//FxMySockMgr::Instance()->CheckDelayRelease();

    return bRet;
}

void FxNetModule::Release()
{
    Uninit();
}

FxIoThread* FxNetModule::FetchIoThread(UINT32 dwSockId)
{
    int nIndex = dwSockId % m_nNetThreadCount;
    return m_pEpollHandlers + nIndex;
}

bool FxNetModule::SetNetOpt(ENetOpt eOpt, int nValue)
{
    if (m_bInit)
    {
        return false;
    }

    switch (eOpt)
    {
    case ENET_MAX_CONNECTION:
        {
            if (0 >= nValue)
            {
                return false;
            }
            m_nMaxConnectionCount = nValue;
        }
        break;

    case ENET_MAX_TOTALEVENT:
        {
            if (0 >= nValue)
            {
                return false;
            }
            m_nTotalEventCount = nValue;
        }
        break;

    default:
        return false;
    }

    return true;
}

