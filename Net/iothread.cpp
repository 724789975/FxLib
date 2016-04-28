#include "iothread.h"
#include "mysock.h"
#include <errno.h>
#include <fcntl.h>
#include "net.h"

#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

FxIoThread::FxIoThread()
{
	m_poThrdHandler = NULL;
#ifdef WIN32
#else
	m_pEvents = NULL;
	m_hEpoll = INVALID_SOCKET;
#endif // WIN32
	m_dwMaxSock = 0;
	m_bStop = false;
}

FxIoThread::~FxIoThread()
{
#ifdef WIN32
	// todo
#else
	if (m_hEpoll != (int) INVALID_SOCKET)
	{
		close(m_hEpoll);
		m_hEpoll = INVALID_SOCKET;
	}

	if (m_pEvents != NULL)
	{
		delete m_pEvents;
		m_pEvents = NULL;
	}
#endif // WIN32
}

bool FxIoThread::Init(UINT32 dwMaxSock)
{
#ifdef WIN32
	// ��������ɶ˿�// �������Ҫ���̺߳���ʼ����֮ǰ����//
	if (GetHandle() == NULL)
	{
		return false;
	}
#else
	m_sockQueue.Init(dwMaxSock);

	m_dwMaxSock = dwMaxSock;
	m_pEvents = new epoll_event[dwMaxSock];
	if (NULL == m_pEvents)
	{
		return false;
	}

	m_hEpoll = epoll_create(dwMaxSock);
	if (m_hEpoll < 0)
	{
		return false;
	}
#endif // WIN32
	if (!Start())
	{
		return false;
	}

	return true;
}

void FxIoThread::Uninit()
{
	if (!m_bStop)
	{
		Stop();
	}
#ifdef WIN32
#else
	if (m_hEpoll != (int) INVALID_SOCKET)
	{
		close(m_hEpoll);
		m_hEpoll = INVALID_SOCKET;
	}

	if (m_pEvents != NULL)
	{
		delete[] m_pEvents;
		m_pEvents = NULL;
	}
#endif // WIN32
}

void FxIoThread::__DealEpollSock()
{
//#ifdef WIN32
//#else
	IFxSocket** ppSock = m_sockQueue.PopFront();
	while (ppSock)
	{
		IFxSocket* poSock = *ppSock;
		if (NULL == poSock)
		{
			continue;
		}
//		LogScreen("event socket addr %d, socket id %d", poSock, poSock->GetSockId());
		while (!FxNetModule::Instance()->PushNetEvent(poSock))
		{
			FxSleep(1);
		}
//		poSock->ProcEvent();

		ppSock = m_sockQueue.PopFront();
	}
//#endif	//WIN32
}

#ifdef WIN32
bool FxIoThread::AddEvent(int hSock, IFxSocket* poSock)
{
	if (hSock < 0)
	{
		LogScreen(LogLv_Error, "hSock : %d", hSock);
		LogFile(LogLv_Error, "hSock : %d", hSock);
		return false;
	}

	if (NULL == GetHandle())
	{
		LogScreen(LogLv_Error, "%s", "GetHandle failed");
		LogFile(LogLv_Error, "%s", "GetHandle failed");
		return false;
	}

	if (NULL == CreateIoCompletionPort((HANDLE)hSock, GetHandle(), (ULONG_PTR)poSock, 0))
	{
		int dwErr = WSAGetLastError();
		LogScreen(LogLv_Error, "errno %d", dwErr);
		LogFile(LogLv_Error, "errno %d", dwErr);
		return false;
	}

	return true;
}
#else
bool FxIoThread::AddEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock)
{
	if (hSock < 0)
	{
		LogScreen(LogLv_Error, "hSock : %d", hSock);
		LogFile(LogLv_Error, "hSock : %d", hSock);
		return false;
	}

	if (m_hEpoll < 0)
	{
		LogScreen(LogLv_Error, "%s", "m_hEpoll < 0");
		LogFile(LogLv_Error, "%s", "m_hEpoll < 0");
		return false;
	}

	INT32 nFlags = fcntl(hSock, F_GETFL, 0);
	nFlags |= O_NONBLOCK;
	fcntl(hSock, F_SETFL, nFlags);

	epoll_event e;
	e.events = dwEvents;
	e.data.ptr = poSock;

	if (epoll_ctl(m_hEpoll, EPOLL_CTL_ADD, hSock, &e) < 0)
	{
		return false;
	}

	return true;
}

bool FxIoThread::ChangeEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock)
{
	if (m_hEpoll < 0)
	{
		LogScreen(LogLv_Error, "%s", "m_hEpoll < 0");
		LogFile(LogLv_Error, "%s", "m_hEpoll < 0");
		return false;
	}

	if (hSock < 0)
	{
		LogScreen(LogLv_Error, "socket : %d", hSock);
		LogFile(LogLv_Error, "socket : %d", hSock);
		return false;
	}

	epoll_event e;
	e.events = dwEvents;
	e.data.ptr = poSock;

	if (epoll_ctl(m_hEpoll, EPOLL_CTL_MOD, hSock, &e) < 0)
	{
		LogScreen(LogLv_Error, "epoll_ctl errno : %d", errno);
		LogFile(LogLv_Error, "epoll_ctl errno : %d", errno);
		return false;
	}

	return true;
}

bool FxIoThread::DelEvent(int hSock)
{
	if (m_hEpoll < 0)
	{
		return false;
	}

	if (hSock < 0)
	{
		return false;
	}

	epoll_event e;
	if (epoll_ctl(m_hEpoll, EPOLL_CTL_DEL, hSock, &e) < 0)
	{
		return false;
	}
//	LogScreen(LogLv_Error, "%d", hSock);
	return true;
}

#endif // WIN32

void FxIoThread::ThrdFunc()
{
	LogScreen(LogLv_Info, "thread id %d start", m_poThrdHandler->GetThreadId());
	while (!m_bStop)
	{
		if (!__DealEpollData())
		{
			break;
		}

		__DealEpollSock();

		FxSleep(1);
	}
	LogScreen(LogLv_Info, "thread id %d end", m_poThrdHandler->GetThreadId());
}

bool FxIoThread::PushSock(IFxSocket* poSock)
{
    // ���20�λ����޷����ȥ���Ǿ���Ϊʧ��
    if (NULL == poSock)
    {
        return false;
    }

    int nTime =0;
    while (!m_sockQueue.PushBack(poSock))
    {
        if (20 < nTime++)
        {
            return false;
        }
        FxSleep(1);
    }

    return true;
}

bool FxIoThread::__DealEpollData()
{
#ifdef WIN32
	SPerIoData* pstPerIoData = NULL;
	IFxSocket* poSock = NULL;
	BOOL bRet = false;
	DWORD dwByteTransferred = 0;

	while (true)
	{
		poSock = NULL;
		pstPerIoData = NULL;
		dwByteTransferred = 0;

		bRet = GetQueuedCompletionStatus(
				GetHandle(),
				&dwByteTransferred,
				(LPDWORD)&poSock,
				(LPOVERLAPPED*)&pstPerIoData,
				INFINITE);

		// ����Ƿ����߳��˳�
		if (NULL == poSock)
		{
			return false;
		}

		poSock->OnParserIoEvent((FALSE != bRet), pstPerIoData, dwByteTransferred);
	}
#else
	INT32 nCount = WaitEvents(1);
	if (nCount < 0)
	{
		return false;
	}

	for (INT32 i = 0; i < nCount; i++)
	{
		epoll_event* pEvent = GetEvent(i);
		if (NULL == pEvent)
		{
			return false;
		}

		IFxSocket* poSock = (IFxSocket*) pEvent->data.ptr;
		if (NULL == poSock)
		{
			return false;
		}

		poSock->OnParserIoEvent(pEvent->events);
	}

	return true;
#endif // WIN32
}

void FxIoThread::Stop()
{
	m_bStop = true;
	if (m_poThrdHandler != NULL)
	{
		m_poThrdHandler->WaitFor(3000);
		m_poThrdHandler->Release();
		m_poThrdHandler = NULL;
	}
}

bool FxIoThread::Start()
{
	m_poThrdHandler = FxCreateThreadHandler(this, true);
	if (NULL == m_poThrdHandler)
	{
		return false;
	}
	return true;
}

#ifdef WIN32
HANDLE FxIoThread::GetHandle()
{
	// �����Ĺ��Ҫ��֤�ڵ��߳���ִ�� ��ǳ���ֹͣ���� ��Ȼ���һֱ��Ч
	static const HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	return hCompletionPort;
}
#else
int FxIoThread::GetHandle()
{
	return m_hEpoll;
}

int FxIoThread::WaitEvents(int nMilliSecond)
{
	if ((int) INVALID_SOCKET == m_hEpoll)
	{
		return 0;
	}

	int nCount = epoll_wait(m_hEpoll, m_pEvents, m_dwMaxSock, nMilliSecond);
	if (nCount < 0)
	{
		if (errno != EINTR)
		{
			return nCount;
		}
		else
		{
			return 0;
		}
	}
	return nCount;
}

epoll_event* FxIoThread::GetEvent(int nIndex)
{
	if (0 > nIndex || (int) m_dwMaxSock <= nIndex)
	{
		return NULL;
	}

	return &m_pEvents[nIndex];
}

#endif // WIN32

