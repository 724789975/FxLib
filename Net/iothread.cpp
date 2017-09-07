#include "iothread.h"
#include "mytcpsock.h"
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
	m_pFile = NULL;
#ifdef WIN32
#else
	m_pEvents = NULL;
	m_hEpoll = INVALID_SOCKET;
#endif // WIN32
	m_dwMaxSock = 0;
	m_bStop = false;
	sprintf(m_szLogPath, "./%s_%p_log.txt", GetExeName(), this);
}

FxIoThread::~FxIoThread()
{
#ifdef WIN32
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
	// 初始化的时候 先获取下 创建完成端口 //
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (m_hCompletionPort == NULL)
	{
		return false;
	}
#else
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
	m_oDelayCloseSockQueue.Init(2 * dwMaxSock);
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


UINT32 FxIoThread::GetThreadId()
{
	if (m_poThrdHandler)
	{
		return m_poThrdHandler->GetThreadId();
	}
	return 0;
}

void FxIoThread::__DealEpollSock()
{
}

#ifdef WIN32
bool FxIoThread::AddEvent(int hSock, IFxSocket* poSock)
{
	if (hSock < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "hSock : %d, socket id : %d", hSock, poSock->GetSockId());
		return false;
	}

	if (NULL == GetHandle())
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "%s", "GetHandle failed");
		return false;
	}

	if (NULL == CreateIoCompletionPort((HANDLE)hSock, GetHandle(), (ULONG_PTR)poSock, 0))
	{
		int dwErr = WSAGetLastError();
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "CreateIoCompletionPort errno %d", dwErr);
		return false;
	}

	return true;
}
#else
bool FxIoThread::AddEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock)
{
	if (hSock < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "hSock : %d, socket id : %d", hSock, poSock->GetSockId());
		return false;
	}

	if (m_hEpoll < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "%s", "m_hEpoll < 0");
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
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "epoll_ctl errno %d", errno);
		return false;
	}

	return true;
}

bool FxIoThread::ChangeEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock)
{
	if (m_hEpoll < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "%s", "m_hEpoll < 0");
		return false;
	}

	if (hSock < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "socket : %d", hSock);
		return false;
	}

	epoll_event e;
	e.events = dwEvents;
	e.data.ptr = poSock;

	if (epoll_ctl(m_hEpoll, EPOLL_CTL_MOD, hSock, &e) < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "epoll_ctl errno : %d", errno);
		return false;
	}

	return true;
}

bool FxIoThread::DelEvent(int hSock)
{
	if (m_hEpoll < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "%s", "m_hEpoll < 0");
		return false;
	}

	if (hSock < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "socket : %d", hSock);
		return false;
	}

	epoll_event e;
	if (epoll_ctl(m_hEpoll, EPOLL_CTL_DEL, hSock, &e) < 0)
	{
		ThreadLog(LogLv_Error, GetFile(), GetLogFile(), "epoll_ctl errno : %d", errno);
		return false;
	}
	return true;
}

#endif // WIN32

void FxIoThread::ThrdFunc()
{
	ThreadLog(LogLv_Info, GetFile(), GetLogFile(), "thread id %d start", m_poThrdHandler->GetThreadId());
	int i = 0;
	while (!m_bStop)
	{
		if (!__DealEpollData())
		{
			break;
		}

		__DealEpollSock();

		ThreadLog(LogLv_Info, GetFile(), GetLogFile(), "seq %d in %d run!!!!!", i++, m_poThrdHandler->GetThreadId());
		FxSleep(1);
	}
	ThreadLog(LogLv_Info, GetFile(), GetLogFile(), "thread id %d end", m_poThrdHandler->GetThreadId());
}

bool FxIoThread::__DealEpollData()
{
#ifdef WIN32
	void* pstPerIoData = NULL;
	IFxSocket* poSock = NULL;
	BOOL bRet = false;
	DWORD dwByteTransferred = 0;

	//while (true)
	{
		poSock = NULL;
		pstPerIoData = NULL;
		dwByteTransferred = 0;

		bRet = GetQueuedCompletionStatus(
			GetHandle(),
			&dwByteTransferred,
			(PULONG_PTR)&poSock,
			(LPOVERLAPPED*)&pstPerIoData,
			INFINITE);

		// 
		if (NULL == poSock)
		{
			return false;
		}

		poSock->OnParserIoEvent((FALSE != bRet), pstPerIoData, dwByteTransferred);
	}
#else
	while (true)
	{
		IFxSocket** ppSock = m_oDelayCloseSockQueue.PopFront();
		if (ppSock == NULL)
		{
			break;
		}

		IFxSocket* poSock = *ppSock;
		if (poSock == NULL)
		{
			break;
		}
		poSock->Close();
	}

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
#endif // WIN32
	return true;
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
	FxCreateThreadHandler(this, true, m_poThrdHandler);
	if (NULL == m_poThrdHandler)
	{
		return false;
	}
	return true;
}

#ifdef WIN32
HANDLE FxIoThread::GetHandle()
{
	// 创建完成端口
	return m_hCompletionPort;
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


void FxIoThread::PushDelayCloseSock(IFxSocket* poSock)
{
	while (!m_oDelayCloseSockQueue.PushBack(poSock))
	{
		FxSleep(10);
	}
}

#endif // WIN32

