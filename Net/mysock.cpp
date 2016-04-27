#include "mysock.h"
#include "sockmgr.h"
#include <stdio.h>
#include "connectionmgr.h"
#include "connection.h"
#include "iothread.h"
#include "net.h"

#ifdef WIN32
struct tcp_keepalive {
	u_long  onoff;
	u_long  keepalivetime;
	u_long  keepaliveinterval;
};
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // !_WINSOCK_DEPRECATED_NO_WARNINGS
#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)
#else
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <execinfo.h>
#include <pthread.h>
int     MAX_SYS_SEND_BUF = (128 * 1024);
int     VAL_SO_SNDLOWAT = (64 * 1024);
#endif // WIN32

int nAccept = 0;
FxListenSock::FxListenSock()
{
	// 千万不要在这里有给指针赋值的操作 因为 会复制构造//
	Reset();

	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	//m_dwLastError = 0;
	//m_bSendLinger = false;     // 发送延迟，直到成功，或者30次后，这时默认设置//
	m_poSessionFactory = NULL;
}


FxListenSock::~FxListenSock()
{
	if (m_poSessionFactory)
	{
		delete m_poSessionFactory;
		m_poSessionFactory = NULL;
	}
}

bool FxListenSock::Init()
{
	return m_oEvtQueue.Init(MAX_NETEVENT_PERSOCK);
}

void FxListenSock::OnRead()
{
}

void FxListenSock::OnWrite()
{
}

bool FxListenSock::Listen(UINT32 dwIP, UINT16 wPort)
{
	SetSock(socket(AF_INET, SOCK_STREAM, 0));
	if (INVALID_SOCKET == GetSock())
	{

#ifdef WIN32
		LogScreen("create socket error, %u:%u, errno %d", dwIP, wPort, WSAGetLastError());
		LogFile("create socket error, %u:%u, errno %d", dwIP, wPort, WSAGetLastError());
#else
		LogScreen("create socket error, %u:%u, errno %d", dwIP, wPort, errno);
		LogFile("create socket error, %u:%u, errno %d", dwIP, wPort, errno);
#endif // WIN32
		LogFile(PrintTrace());
		return false;
	}

	INT32 nReuse = 1;
	setsockopt(GetSock(), SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof(nReuse));

	sockaddr_in stAddr = { 0 };
	stAddr.sin_family = AF_INET;
	if (0 == dwIP)
	{
		stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		stAddr.sin_addr.s_addr = dwIP;
	}
	stAddr.sin_port = htons(wPort);

	if (bind(GetSock(), (sockaddr*)&stAddr, sizeof(stAddr)) < 0)
	{
#ifdef WIN32
		LogScreen("bind at %u:%d failed, errno %d", dwIP, wPort, WSAGetLastError());
		LogFile("bind at %u:%d failed, errno %d", dwIP, wPort, WSAGetLastError());
#else
		LogScreen("bind at %u:%d failed, errno %d", dwIP, wPort, errno);
		LogFile("bind at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32
		LogFile(PrintTrace());

		return false;
	}
	if (listen(GetSock(), 128) < 0)
	{
#ifdef WIN32
		LogScreen("listen at %u:%d failed, errno %d", dwIP, wPort, WSAGetLastError());
		LogFile("listen at %u:%d failed, errno %d", dwIP, wPort, WSAGetLastError());
#else
		LogScreen("listen at %u:%d failed, errno %d", dwIP, wPort, errno);
		LogFile("listen at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32
		LogFile(PrintTrace());
		return false;
	}

	m_poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(GetSockId());
	if (NULL == m_poIoThreadHandler)
	{
		Close();
		return false;
	}
	// 加入线程处理//
	if (false == AddEvent())
	{
		return false;
	}
#ifdef WIN32

	if (false == InitAcceptEx())
	{
		LogScreen("CCpListener::Start, InitAcceptEx failed");
		LogFile("CCpListener::Start, InitAcceptEx failed");
		LogFile(PrintTrace());
		return false;
	}

	for (INT32 i = 0; i < sizeof(m_oSPerIoDatas) / sizeof(m_oSPerIoDatas[0]); i++)
	{
		if (false == PostAccept(m_oSPerIoDatas[i]))
		{
			LogScreen("CCpListener::Start, PostAccept failed");
			LogFile("CCpListener::Start, PostAccept failed");
			LogFile(PrintTrace());
			return false;
		}
	}
#else
#endif // WIN32

	SetState(SSTATE_LISTEN);

	LogScreen("listen at %u:%d success\n", dwIP, wPort);
	return true;
}

bool FxListenSock::StopListen()
{
	if (SSTATE_LISTEN != GetState())
	{
		LogScreen("state : %d != SSTATE_LISTEN", (UINT32)GetState());
		LogFile("state : %d != SSTATE_LISTEN", (UINT32)GetState());
		LogFile(PrintTrace());
		return false;
	}

	if (INVALID_SOCKET == GetSock())
	{
		LogScreen("socket : %d == INVALID_SOCKET", GetSock());
		LogFile("socket : %d == INVALID_SOCKET", GetSock());
		LogFile(PrintTrace());
		return false;
	}

#ifdef WIN32
	CancelIo((HANDLE)GetSock());
	closesocket(GetSock());
	SetSock(INVALID_SOCKET);
#else

	m_poIoThreadHandler->DelEvent(GetSock());
	close(GetSock());
	SetSock(INVALID_SOCKET);
#endif // WIN32

	SetState(SSTATE_STOP_LISTEN);
	return true;
}

bool FxListenSock::Close()
{
	m_oLock.Lock();
#ifdef WIN32
	closesocket(GetSock());
#else
	m_poIoThreadHandler->DelEvent(GetSock());
	close(GetSock());
#endif // WIN32

	SetSock(INVALID_SOCKET);

	PushNetEvent(NETEVT_TERMINATE, 0);
	m_oLock.UnLock();

	return true;
}

void FxListenSock::Reset()
{
	m_poSessionFactory = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
}

bool FxListenSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{
	if (SSTATE_INVALID == GetState())
	{
		LogScreen("state : %d == SSTATE_INVALID", (UINT32)GetState());
		LogFile("state : %d == SSTATE_INVALID", (UINT32)GetState());
		LogFile(PrintTrace());
		return false;
	}
	SNetEvent oEvent;
	// 先扔网络事件进去，然后在报告上层有事件，先后顺序不能错，这样上层就不会错取事件//
	oEvent.eType = eType;
	oEvent.dwValue = dwValue;
	while (!m_oEvtQueue.PushBack(oEvent))
	{
		FxSleep(1);
	}

	while(!m_poIoThreadHandler->PushSock(this))
	{
		break;
		FxSleep(1);
	}

	return true;
}

bool FxListenSock::AddEvent()
{
#ifdef WIN32
	if (!m_poIoThreadHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		Close();
		return false;
	}
#else
	if (!m_poIoThreadHandler->AddEvent(GetSock(), EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return false;
	}
#endif // WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;

}

void FxListenSock::ProcEvent()
{
	SNetEvent* pEvent = m_oEvtQueue.PopFront();
	if (pEvent)
	{
		switch (pEvent->eType)
		{
		case NETEVT_ASSOCIATE:
		{
			__ProcAssociate();
		}
			break;

		case NETEVT_ERROR:
		{
			__ProcError(pEvent->dwValue);
		}
			break;

		case NETEVT_TERMINATE:
		{
			__ProcTerminate();
		}
			break;

		default:
		{
				   assert(0);
		}
			break;
		}
	}
}

void FxListenSock::__ProcAssociate()
{

}

void FxListenSock::__ProcError(UINT32 dwErrorNo)
{

}

void FxListenSock::__ProcTerminate()
{

}

#ifdef WIN32
bool FxListenSock::PostAccept(SPerIoData& oSPerIoData)
{
	SOCKET hNewSock = WSASocket(
		AF_INET,
		SOCK_STREAM,
		0,
		NULL,
		0,
		WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == hNewSock)
	{
		LogScreen("WSASocket failed, errno %d", WSAGetLastError());
		LogFile("WSASocket failed, errno %d", WSAGetLastError());
		LogFile(PrintTrace());
		return false;
	}

	unsigned long ul = 1;
	if (SOCKET_ERROR == ioctlsocket(hNewSock, FIONBIO, (unsigned long*)&ul))
	{
		closesocket(hNewSock);
		LogScreen("Set socket FIONBIO error : %d", WSAGetLastError());
		LogFile("Set socket FIONBIO error : %d", WSAGetLastError());
		LogFile(PrintTrace());
		return false;
	}

	int nSendBuffSize = 256 * 1024;
	int nRecvBuffSize = 8 * nSendBuffSize;
	if ((0 != setsockopt(hNewSock, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuffSize, sizeof(int))) ||
		(0 != setsockopt(hNewSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuffSize, sizeof(int))))
	{
		int nError = WSAGetLastError();
		closesocket(hNewSock);
		return false;
	}

	memset(&(oSPerIoData.stOverlapped), 0, sizeof(oSPerIoData.stOverlapped));
	oSPerIoData.hSock = hNewSock;
	oSPerIoData.nOp = IOCP_ACCEPT;

	DWORD dwBytes;
	BOOL bRet = m_lpfnAcceptEx(
		GetSock(),
		hNewSock,
		oSPerIoData.Buf,
		0,
		sizeof(SOCKADDR_IN)+16,
		sizeof(SOCKADDR_IN)+16,
		&dwBytes,
		&oSPerIoData.stOverlapped);

	if (false == bRet)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			LogScreen("Init AcceptEx failed, errno %d", WSAGetLastError());
			LogFile("Init AcceptEx failed, errno %d", WSAGetLastError());
			LogFile(PrintTrace());
			closesocket(hNewSock);
			return false;
		}
	}

	return true;
}

bool FxListenSock::InitAcceptEx()
{
	DWORD dwbytes = 0;

	GUID m_GuidAcceptEx = WSAID_ACCEPTEX;

	int nRt = ::WSAIoctl(
		GetSock(),
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&m_GuidAcceptEx,
		sizeof(m_GuidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(LPFN_ACCEPTEX),
		&dwbytes,
		NULL,
		NULL);

	if (SOCKET_ERROR == nRt)
	{
		LogScreen("WSAIoctl WSAID_ACCEPTEX failed, errno %d", WSAGetLastError());
		LogFile("WSAIoctl WSAID_ACCEPTEX failed, errno %d", WSAGetLastError());
		LogFile(PrintTrace());
		return false;
	}

	GUID m_GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

	dwbytes = 0;

	nRt = ::WSAIoctl(
		GetSock(),
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&m_GuidGetAcceptExSockaddrs,
		sizeof(m_GuidGetAcceptExSockaddrs),
		&m_lpfnGetAcceptExSockaddrs,
		sizeof(LPFN_GETACCEPTEXSOCKADDRS),
		&dwbytes,
		NULL,
		NULL);

	if (SOCKET_ERROR == nRt)
	{
		LogScreen("WSAIoctl WSAID_GETACCEPTEXSOCKADDRS failed, errno %d", WSAGetLastError());
		LogFile("WSAIoctl WSAID_GETACCEPTEXSOCKADDRS failed, errno %d", WSAGetLastError());
		LogFile(PrintTrace());
		return false;
	}

	return true;
}

void FxListenSock::OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred)
{
	if (SSTATE_LISTEN == GetState())
	{
		m_oLock.Lock();
		if (false == bRet)
		{
			LogScreen("CCPSock::OnAccept, accept failed, errno %d", WSAGetLastError());
			LogFile("CCPSock::OnAccept, accept failed, errno %d", WSAGetLastError());
			LogFile(PrintTrace());
			closesocket(pIoData->hSock);
			PostAccept(*pIoData);
		}
		OnAccept(pIoData);
		m_oLock.UnLock();
	}
	else
	{
		LogScreen("state : %d != SSTATE_LISTEN", (UINT32)GetState());
		LogFile("state : %d != SSTATE_LISTEN", (UINT32)GetState());
		LogFile(PrintTrace());
		Close();        // 未知错误，不应该发生//
	}
}

void FxListenSock::OnAccept(SPerIoData* pstPerIoData)
{
	SOCKET hSock = pstPerIoData->hSock;

	if (SSTATE_LISTEN != GetState())
	{
		LogScreen("state : %d != SSTATE_LISTEN", GetState());
		LogFile("state : %d != SSTATE_LISTEN", GetState());
		LogFile(PrintTrace());
		closesocket(hSock);
		return;
	}

	{
		FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
		if (NULL == poSock)
		{
			LogScreen("CCPSock::OnAccept, create CCPSock failed");
			LogFile("CCPSock::OnAccept, create CCPSock failed");
			LogFile(PrintTrace());
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxIoThread* poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSockId());
		if (NULL == poIoThreadHandler)
		{
			LogScreen("CCPSock::OnAccept, get iothread failed");
			LogFile("CCPSock::OnAccept, get iothread failed");
			LogFile(PrintTrace());
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
		if (NULL == poConnection)
		{
			LogScreen("CCPSock::OnAccept, create Connection failed");
			LogFile("CCPSock::OnAccept, create Connection failed");
			LogFile(PrintTrace());
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			return;
		}

		::setsockopt(hSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&(GetSock()), sizeof(SOCKET));

		poSock->SetSock(hSock);
		poSock->SetConnection(poConnection);

		poConnection->SetSock(poSock);
		poConnection->SetID(poSock->GetSockId());

		sockaddr_in* pstRemoteAddr = NULL;
		sockaddr_in* pstLocalAddr = NULL;
		INT32 nRemoteAddrLen = sizeof(sockaddr_in);
		INT32 nLocalAddrLen = sizeof(sockaddr_in);
		INT32 nAddrLen = sizeof(sockaddr_in)+16;

		m_lpfnGetAcceptExSockaddrs(
			pstPerIoData->Buf,
			0,
			nAddrLen,
			nAddrLen,
			(SOCKADDR**)&pstLocalAddr,
			&nLocalAddrLen,
			(SOCKADDR**)&pstRemoteAddr,
			&nRemoteAddrLen);

		poConnection->SetLocalIP(pstLocalAddr->sin_addr.s_addr);
		poConnection->SetLocalPort(ntohs(pstLocalAddr->sin_port));

		poConnection->SetRemoteIP(pstRemoteAddr->sin_addr.s_addr);
		poConnection->SetRemotePort(ntohs(pstRemoteAddr->sin_port));

		FxSession* poSession = m_poSessionFactory->CreateSession();
		if (NULL == poSession)
		{
			LogScreen("CCPSock::OnAccept, CreateSession failed");
			LogFile("CCPSock::OnAccept, CreateSession failed");
			LogFile(PrintTrace());
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		poSession->Init(poConnection);
		poConnection->SetSession(poSession);
		poSock->SetIoThread(poIoThreadHandler);

		poSock->SetState(SSTATE_ESTABLISH);

		// keep alive
		struct tcp_keepalive keepAliveIn;
		struct tcp_keepalive keepAliveOut;

		unsigned long ulBytesReturn = 0;

		keepAliveIn.keepaliveinterval = 10000;//
		keepAliveIn.keepalivetime = 1000 * 30;//
		keepAliveIn.onoff = 1;

		int ret = WSAIoctl
			(
			poSock->GetSock(),
			SIO_KEEPALIVE_VALS,
			&keepAliveIn,
			sizeof(keepAliveIn),
			&keepAliveOut,
			sizeof(keepAliveOut),
			&ulBytesReturn,
			NULL,
			NULL
			);
		if (ret == SOCKET_ERROR)
		{
			LogScreen("Set keep alive error: %d", WSAGetLastError());
			LogFile("Set keep alive error: %d", WSAGetLastError());
			LogFile(PrintTrace());
			PostAccept(*pstPerIoData);
			poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
			poSock->Close();
			return;
		}

		//
		// 应该先投递连接事件再关联套接口，否则可能出现第一个Recv事件先于连接事件入队列
		//

		if (false == poSock->AddEvent())
		{
			LogScreen("poSock->AddEvent failed");
			LogFile("poSock->AddEvent failed");
			LogFile(PrintTrace());
			poSock->Close();
		}
		else
		{
			poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

			if (false == poSock->PostRecv())
			{
				poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
				LogScreen("poSock->PushNetEvent failed, errno : %d", WSAGetLastError());
				LogFile("poSock->PushNetEvent failed, errno : %d", WSAGetLastError());
				LogFile(PrintTrace());
				poSock->Close();
			}
		}

		PostAccept(*pstPerIoData);
	}
}

#else
void FxListenSock::OnParserIoEvent(int dwEvents)
{
	if (dwEvents & EPOLLERR)
	{
		PushNetEvent(NETEVT_ERROR, errno);
		LogScreen("get error event errno : %d", errno);
		LogFile("get error event errno : %d", errno);
		LogFile(PrintTrace());
		Close();
		return;
	}

	if (dwEvents & EPOLLIN)
	{
		OnAccept();
	}
}

void FxListenSock::OnAccept()
{
	sockaddr_in stLocalAddr;
	sockaddr_in stRemoteAddr;
	UINT32 dwAddrLen = sizeof(stRemoteAddr);
	UINT32 hAcceptSock = accept(GetSock(), (sockaddr*)&stRemoteAddr, &dwAddrLen);
	if (INVALID_SOCKET == hAcceptSock)
	{
		LogScreen("%s", "INVALID_SOCKET == hAcceptSock");
		LogFile("%s", "INVALID_SOCKET == hAcceptSock");
		LogFile(PrintTrace());
		return;
	}

	FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
	if (NULL == poSock)
	{
		LogScreen("%s", "create FxConnectSock failed");
		LogFile("%s", "create FxConnectSock failed");
		LogFile(PrintTrace());
		close(hAcceptSock);
		return;
	}

	FxIoThread* poEpollHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSockId());
	if (NULL == poEpollHandler)
	{
		LogScreen("%s", "NULL == poEpollHandler");
		LogFile("%s", "NULL == poEpollHandler");
		LogFile(PrintTrace());
		close(hAcceptSock);
		return;
	}

	FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
	if (NULL == poConnection)
	{
		LogScreen("%s", "NULL == poConnection");
		LogFile("%s", "NULL == poConnection");
		LogFile(PrintTrace());
		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		return;
	}

	FxSession* poSession = m_poSessionFactory->CreateSession();
	if (NULL == poSession)
	{
		LogScreen("%s", "NULL == poSession");
		LogFile("%s", "NULL == poSession");
		LogFile(PrintTrace());
		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		FxConnectionMgr::Instance()->Release(poConnection);
		return;
	}

	setsockopt(GetSock(), SOL_SOCKET, SO_SNDLOWAT, &VAL_SO_SNDLOWAT, sizeof(VAL_SO_SNDLOWAT));
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, &MAX_SYS_SEND_BUF, sizeof(MAX_SYS_SEND_BUF));

	dwAddrLen = sizeof(stLocalAddr);
	getsockname(hAcceptSock, (sockaddr*)&stLocalAddr, &dwAddrLen);

	poSession->Init(poConnection);
	poConnection->SetID(poSock->GetSockId());
	poConnection->SetSock(poSock);
	poConnection->SetSession(poSession);

	poConnection->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	poConnection->SetLocalPort(ntohs(stLocalAddr.sin_port));
	poConnection->SetRemoteIP(stRemoteAddr.sin_addr.s_addr);
	poConnection->SetRemotePort(ntohs(stRemoteAddr.sin_port));

	poSock->SetSock(hAcceptSock);
	poSock->SetConnection(poConnection);
	poSock->SetIoThread(poEpollHandler);

	poSock->SetState(SSTATE_ESTABLISH);

	// keep alive
	int keepAlive = 1;
	setsockopt(GetSock(), SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));

	int keepIdle = 30;
	int keepInterval = 5;
	int keepCount = 6;
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	if(!poSock->AddEvent())
	{
		LogScreen("%s", "poSock->AddEvent() failed");
		LogFile("%s", "poSock->AddEvent() failed");
		LogFile(PrintTrace());
		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);

		poSession->Release();
		return;
	}
	poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

	return;
}

#endif // WIN32

/************************************************************************/
/*                                                                      */
/************************************************************************/

FxConnectSock::FxConnectSock()
{
	//m_pListenSocket = NULL;
	m_dwLastError = 0;
	m_poSendBuf = NULL;
	m_poRecvBuf = NULL;
	m_poConnection = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_nNeedData = 0;
	m_nPacketLen = 0;
#ifdef WIN32
	m_stRecvIoData.nOp = IOCP_RECV;
	m_stSendIoData.nOp = IOCP_SEND;
	m_nPostRecv = 0;
	m_nPostSend = 0;
	m_dwLastError = 0;
#else
	m_bSending = false;
#endif // WIN32

	m_bSendLinger = false;     // 发送延迟，直到成功，或者30次后，这时默认设置//
	m_oEvtQueue.Init(MAX_NETEVENT_PERSOCK);

	Reset();
}

FxConnectSock::~FxConnectSock()
{
	if (m_poConnection)
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		m_poConnection->OnSocketDestroy();
		// 让session去负责release吧 只要session存在 那么就肯定有connection//
		m_poConnection = NULL;
	}

	if (m_poRecvBuf)
	{
		FxLoopBuffMgr::Instance()->Release(m_poRecvBuf);
		m_poRecvBuf = NULL;
	}
	if (m_poSendBuf)
	{
		FxLoopBuffMgr::Instance()->Release(m_poSendBuf);
		m_poSendBuf = NULL;
	}

}

bool FxConnectSock::Init()
{
	if (!m_oEvtQueue.Init(MAX_NETEVENT_PERSOCK))
	{
		LogScreen("%s", "m_oEvtQueue.Init failed");
		LogFile("%s", "m_oEvtQueue.Init failed");
		LogFile(PrintTrace());
		return false;
	}

	if (NULL == m_poSendBuf)
	{
		m_poSendBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poSendBuf)
		{
			LogScreen("%s", "NULL == m_poSendBuf");
			LogFile("%s", "NULL == m_poSendBuf");
			LogFile(PrintTrace());
			return false;
		}
	}

	if (NULL == m_poRecvBuf)
	{
		m_poRecvBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poRecvBuf)
		{
			LogScreen("%s", "NULL == m_poRecvBuf");
			LogFile("%s", "NULL == m_poRecvBuf");
			LogFile(PrintTrace());
			return false;
		}
	}

	if (!m_poRecvBuf->Init(RECV_BUFF_SIZE))
	{
		LogScreen("%s", "m_poRecvBuf->Init failed");
		LogFile("%s", "m_poRecvBuf->Init failed");
		LogFile(PrintTrace());
		return false;
	}

	if (!m_poSendBuf->Init(SEND_BUFF_SIZE))
	{
		LogScreen("%s", "m_poSendBuf->Init failed");
		LogFile("%s", "m_poSendBuf->Init failed");
		LogFile(PrintTrace());
		return false;
	}
	return true;
}

void FxConnectSock::OnRead()
{
}

void FxConnectSock::OnWrite()
{
}

bool FxConnectSock::Close()
{
	// 首先 把数据先发过去//
	m_oLock.Lock();

	if(GetState() == SSTATE_RELEASE || GetState() == SSTATE_CLOSE)
	{
		m_oLock.UnLock();
		return true;
//		assert(0);
	}

	if (IsConnect())
	{
		SetState(SSTATE_CLOSE);
	}
	if (GetSock() == INVALID_SOCKET)
	{
		m_oLock.UnLock();
		return true;
	}

#ifdef WIN32
	shutdown(GetSock(), SD_RECEIVE);
#else
	shutdown(GetSock(), SHUT_RD);
	m_poIoThreadHandler->DelEvent(GetSock());
	// 有bug Windows下 就不发了 修改后再说//
	SendImmediately();
#endif	//WIN32

#ifdef WIN32
	if (0 != m_dwLastError)
	{
		PushNetEvent(NETEVT_ERROR, m_dwLastError);
		m_dwLastError = 0;
	}
#endif // WIN32

#ifdef WIN32
	closesocket(GetSock());
#else
	close(GetSock());
#endif // WIN32

	SetSock(INVALID_SOCKET);

	PushNetEvent(NETEVT_TERMINATE, 0);
	m_oLock.UnLock();

	return true;
}

void FxConnectSock::Reset()
{
	m_dwLastError = 0;
	if (NULL != m_poConnection)
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		m_poConnection->OnSocketDestroy();
		// 让session去负责release吧 只要session存在 那么就肯定有connection//
		SetConnection(NULL);
	}

	//m_pListenSocket = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_nNeedData = 0;
	m_nPacketLen = 0;

	if (m_poRecvBuf)
	{
		FxLoopBuffMgr::Instance()->Release(m_poRecvBuf);
		m_poRecvBuf = NULL;
	}
	if (m_poSendBuf)
	{
		FxLoopBuffMgr::Instance()->Release(m_poSendBuf);
		m_poSendBuf = NULL;
	}

#ifdef WIN32
	m_nPostRecv = 0;
	m_nPostSend = 0;
	m_dwLastError = 0;
#else
	m_bSending = false;
#endif // WIN32
	m_bSendLinger = false;     // 发送延迟，直到成功，或者30次后，这时默认设置//
}

// win下出现问题时 要投递关闭信息 因为需要一个post关闭的过程//
bool FxConnectSock::Send(const char* pData, int dwLen)
{
	if (false == IsConnect())
	{
		LogScreen("%s", "socket not connected");
		LogFile("%s", "socket not connected");
		LogFile(PrintTrace());
		return false;
	}

	if (GetState() != SSTATE_ESTABLISH)
	{
		LogScreen("socket state : %d != SSTATE_ESTABLISH", (UINT32)GetState());
		LogFile("socket state : %d != SSTATE_ESTABLISH", (UINT32)GetState());
		LogFile(PrintTrace());
		return false;
	}

	if (m_poSendBuf->IsEmpty())
	{
		m_poSendBuf->Clear();
	}

	if (dwLen > m_poSendBuf->GetTotalLen())
	{
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogScreen("send error NET_SEND_OVERFLOW");
		LogFile("send error NET_SEND_OVERFLOW");
		LogFile(PrintTrace());
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogScreen("send error NET_SEND_OVERFLOW");
		LogFile("send error NET_SEND_OVERFLOW");
		LogFile(PrintTrace());
		Close();
#endif // WIN32
		return false;
	}

	int nSendCount = 0;
	IFxDataHeader* pDataHeader = GetDataHeader();
	if (pDataHeader == NULL)
	{
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogScreen("send error pDataHeader == NULL");
		LogFile("send error pDataHeader == NULL");
		LogFile(PrintTrace());
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogScreen("send error pDataHeader == NULL");
		LogFile("send error pDataHeader == NULL");
		LogFile(PrintTrace());
		Close();
#endif // WIN32
		return false;
	}
	while (!m_poSendBuf->PushBuff((char*)(pDataHeader->BuildSendPkgHeader(dwLen)), pDataHeader->GetHeaderLength()))
	{
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			return false;
		}
		FxSleep(1);
	}

	while (!m_poSendBuf->PushBuff(pData, dwLen))
	{
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			return false;
		}
		FxSleep(1);
	}

	if (false == PostSendFree())
	{
#ifdef WIN32
		m_dwLastError = WSAGetLastError();
		LogScreen("%s", "false == PostSendFree()");
		LogFile("%s", "false == PostSendFree()");
		LogFile(PrintTrace());
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogScreen("%s", "false == PostSendFree()");
		LogFile("%s", "false == PostSendFree()");
		LogFile(PrintTrace());
		Close();
#endif // WIN32
		return false;
	}

	return true;
}

bool FxConnectSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{
	SNetEvent oEvent;
	// 先扔网络事件进去，然后在报告上层有事件，先后顺序不能错，这样上层就不会错取事件//
	oEvent.eType = eType;
	oEvent.dwValue = dwValue;

	while (!m_oEvtQueue.PushBack(oEvent))
	{
		FxSleep(1);
	}

	if(!m_poIoThreadHandler->PushSock(this))
	{
		while (!FxNetModule::Instance()->PushNetEvent(this))
		{
			FxSleep(1);
		}
	}

	return true;
}

bool FxConnectSock::PostSend()
{
#ifdef WIN32
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return false;
	}

	LONG nPostSend = InterlockedCompareExchange(&m_nPostSend, 1, 0);
	if (0 != nPostSend)
	{
		return true;
	}

	ZeroMemory(&m_stSendIoData.stOverlapped, sizeof(m_stSendIoData.stOverlapped));
	int nLen = m_poSendBuf->GetOutCursorPtr(m_stSendIoData.stWsaBuf.buf);
	if (0 >= nLen)
	{
		// 不算失败，只是没有投递而已，下次可以继续//
		InterlockedCompareExchange(&m_nPostSend, 0, 1);
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;     // ???64K

	m_stSendIoData.stWsaBuf.len = nLen;
	DWORD dwNumberOfBytesSent = 0;

	int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0, &m_stSendIoData.stOverlapped, NULL);
	if (0 != nRet)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			InterlockedCompareExchange(&m_nPostSend, 0, 1);

			UINT32 dwErr = WSAGetLastError();
			LogScreen("WSASend errno : %d", WSAGetLastError());
			LogFile("WSASend errno : %d", WSAGetLastError());
			LogFile(PrintTrace());
			return false;
		}
	}

	return true;
#else
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		LogScreen("%s", "NULL == m_poIoThreadHandler");
		LogFile("%s", "NULL == m_poIoThreadHandler");
		LogFile(PrintTrace());
		Close();
		return false;
	}

	if (m_bSending)
	{
		return true;
	}

	char* pSendBuf = NULL;
	int nLen = m_poSendBuf->GetOutCursorPtr(pSendBuf);
	if (0 >= nLen || NULL == pSendBuf)
	{
		// 不算失败，只是没有投递而已，下次可以继续//
		// [12-03-16] hum modify: 在这里需要设置事件为EPOLLIN，否则OUT事件一直被设置，导致CPU很高//
		if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
		{
			LogScreen("%s", "false == m_poIoThreadHandler->ChangeEvent");
			LogFile("%s", "false == m_poIoThreadHandler->ChangeEvent");
			LogFile(PrintTrace());
			PushNetEvent(NETEVT_ERROR, errno);
			Close();
			return false;
		}
		return true;
	}

	m_bSending = true;
	nLen = VAL_SO_SNDLOWAT < nLen ? VAL_SO_SNDLOWAT : nLen;     // 最大64K

	int nRet = send(GetSock(), pSendBuf, nLen, 0);
	if (0 > nRet)
	{
		if (EAGAIN == errno)
		{
			if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
			{
				LogScreen("%s", "false == m_poIoThreadHandler->ChangeEvent");
				LogFile("%s", "false == m_poIoThreadHandler->ChangeEvent");
				LogFile(PrintTrace());
				m_bSending = false;
				return false;
			}
			return true;
		}

		LogScreen("%s", "0 > nRet");
		LogFile("%s", "0 > nRet");
		LogFile(PrintTrace());
		m_bSending = false;
		return false;
	}
	else if (0 == nRet)
	{
		LogScreen("%s", "0 == nRet");
		LogFile("%s", "0 == nRet");
		LogFile(PrintTrace());
		m_bSending = false;
		return false;
	}

	// 把成功发送了的从发送缓冲区丢弃。//
	m_poSendBuf->DiscardBuff(nRet);

	if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		LogScreen("%s", "false == m_poIoThreadHandler->ChangeEvent");
		LogFile("%s", "false == m_poIoThreadHandler->ChangeEvent");
		LogFile(PrintTrace());
		m_bSending = false;
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return false;
	}

	return true;
#endif // WIN32

}

bool FxConnectSock::PostSendFree()
{
#ifdef WIN32
	return PostSend();
#else
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		LogScreen("%s", "NULL == m_poIoThreadHandler");
		LogFile("%s", "NULL == m_poIoThreadHandler");
		LogFile(PrintTrace());
		Close();
		return false;
	}

	return m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this);

	return true;
#endif // WIN32
}
bool FxConnectSock::SendImmediately()
{
	if (GetState() != SSTATE_CLOSE)
	{
		return false;
	}

#ifdef WIN32
	if (m_dwLastError)
#else
	if(errno != 0)
#endif	//WIN32
	{
		// 已经出错了 不能发送//
		return true;
	}

	if (NULL == m_poIoThreadHandler)
	{
//		Close();
		return false;
	}

	while (true)
	{
#ifdef WIN32
		LONG nPostSend = InterlockedCompareExchange(&m_nPostSend, 1, 0);
		if (0 != nPostSend)
		{
			FxSleep(10);
		}
		else
		{
			break;
		}
#else
		if (m_bSending)
		{
			FxSleep(10);
		}
		else
		{
			break;
		}
#endif // WIN32
	}

	while (true)
	{
		if (false == IsConnect())
		{
			return false;
		}

		char* pSendBuf = NULL;
		int nLen = m_poSendBuf->GetOutCursorPtr(pSendBuf);
		if (0 >= nLen || NULL == pSendBuf)
		{
			return true;
		}

#ifdef WIN32
		InterlockedCompareExchange(&m_nPostSend, 0, 1);
#else
		m_bSending = true;
#endif // WIN32

		nLen = 64 * 1024 < nLen ? 64 * 1024 : nLen;     // 最大64K

		int nRet = send(GetSock(), pSendBuf, nLen, 0);
		if (0 > nRet)
		{
			UINT32 dwError = 0;
#ifdef WIN32
			dwError = WSAGetLastError();
			InterlockedCompareExchange(&m_nPostSend, m_nPostSend - 1, m_nPostSend);
#else
			m_bSending = false;
			dwError = errno;
#endif // WIN32
			//continue;
			if (dwError)
			{
				return false;
			}
			else
			{
				continue;
			}
			//return false;	//前面已经等待了一段时间 这个时候要是发不过去 可能是客户端那边已经断了 发不过去了//
		}
		else if (0 == nRet)
		{
			return true;
		}

		// 把成功发送了的从发送缓冲区丢弃。//
		m_poSendBuf->DiscardBuff(nRet);
	}
	return true;
}

void FxConnectSock::__ProcEstablish()
{
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		m_poConnection->OnConnect();
	}
}

void FxConnectSock::__ProcAssociate()
{
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		m_poConnection->OnAssociate();
	}
}

void FxConnectSock::__ProcConnectError(UINT32 dwErrorNo)
{
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		m_poConnection->OnConnError(dwErrorNo);
	}
}

void FxConnectSock::__ProcError(UINT32 dwErrorNo)
{
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		m_poConnection->OnError(dwErrorNo);
	}
}

int nTerminate = 0;
void FxConnectSock::__ProcTerminate()
{
	PushNetEvent(NETEVT_RELEASE, 0);
}

void FxConnectSock::__ProcRecv(UINT32 dwLen)
{
	if (m_poConnection)
	{
		if (UINT32(-1) == dwLen)
		{
			PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
			Close();
			return;
		}

		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		if (m_poConnection->GetRecvSize() < dwLen)
		{
			PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
			Close();
			return;
		}

		if (!m_poRecvBuf->PopBuff(m_poConnection->GetRecvBuf(), dwLen))
		{
			return;
		}
		m_poConnection->OnRecv(dwLen);
	}
}

void FxConnectSock::__ProcRelease()
{
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
			return;
		}

		m_poConnection->OnSocketDestroy();
		m_poConnection->OnClose();

		SetConnection(NULL);
	}
	FxMySockMgr::Instance()->Release(this);
}

IFxDataHeader* FxConnectSock::GetDataHeader()
{
	if (m_poConnection)
	{
		return m_poConnection->GetDataHeader();
	}
	return NULL;
}

bool FxConnectSock::AddEvent()
{
#ifdef WIN32
	if (!m_poIoThreadHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		LogScreen("error : %d", WSAGetLastError());
		LogFile("error : %d", WSAGetLastError());
		LogFile(PrintTrace());
		Close();
		return false;
	}
#else
	if (!m_poIoThreadHandler->AddEvent(GetSock(), EPOLLOUT|EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		LogScreen("error : %d", errno);
		LogFile("error : %d", errno);
		LogFile(PrintTrace());
		Close();
		return false;
	}
#endif // WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;

}

SOCKET FxConnectSock::Connect()
{
	if (IsConnect())
	{
		return INVALID_SOCKET;
	}

	if (NULL == m_poConnection)
	{
		return INVALID_SOCKET;
	}

	if (m_poConnection->IsConnected())
	{
		return INVALID_SOCKET;
	}

	if (INVALID_SOCKET != GetSock())
	{
		return INVALID_SOCKET;
	}

#ifdef WIN32
	SetSock(WSASocket(
		AF_INET,
		SOCK_STREAM,
		0,
		NULL,
		0,
		WSA_FLAG_OVERLAPPED));
#else
	SetSock(socket(AF_INET, SOCK_STREAM, 0));
#endif // WIN32

	if (INVALID_SOCKET == GetSock())
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return INVALID_SOCKET;
	}

#ifdef WIN32
	// keep alive
	struct tcp_keepalive keepAliveIn;
	struct tcp_keepalive keepAliveOut;

	unsigned long ulBytesReturn = 0;

	keepAliveIn.keepaliveinterval = 10000;//
	keepAliveIn.keepalivetime = 1000 * 30;//
	keepAliveIn.onoff = 1;

	int ret = WSAIoctl
		(
		GetSock(),
		SIO_KEEPALIVE_VALS,
		&keepAliveIn,
		sizeof(keepAliveIn),
		&keepAliveOut,
		sizeof(keepAliveOut),
		&ulBytesReturn,
		NULL,
		NULL
		);
	if (ret == SOCKET_ERROR)
	{
		LogScreen("Set keep alive error: %d\n", WSAGetLastError());
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		Close();
		return INVALID_SOCKET;
	}

	sockaddr_in local_addr;
	ZeroMemory(&local_addr, sizeof (sockaddr_in));
	local_addr.sin_family = AF_INET;

	int nSendBuffSize = 256 * 1024;
	int nRecvBuffSize = 8 * nSendBuffSize;
	if ((0 != setsockopt(GetSock(), SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuffSize, sizeof(int))) ||
		(0 != setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuffSize, sizeof(int))))
	{
		int nError = WSAGetLastError();
		PushNetEvent(NETEVT_CONN_ERR, nError);
		closesocket(GetSock());
		return INVALID_SOCKET;
	}

	//todo
	//int irt = ::bind(GetSock(), (sockaddr *)(&local_addr), sizeof (sockaddr_in));
#else
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDLOWAT, &VAL_SO_SNDLOWAT, sizeof(VAL_SO_SNDLOWAT));
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, &MAX_SYS_SEND_BUF, sizeof(MAX_SYS_SEND_BUF));

	int keepAlive = 1;
	setsockopt(GetSock(), SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));

	int keepIdle = 30;
	int keepInterval = 5;
	int keepCount = 6;
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(GetSock(), SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
#endif // WIN32
	// [end]

	SetIoThread(FxNetModule::Instance()->FetchIoThread(GetSockId()));
	if (NULL == m_poIoThreadHandler)
	{
//		m_pLock.UnLock();
		PushNetEvent(NETEVT_ERROR, 0);
		Close();
		return INVALID_SOCKET;
	}

	sockaddr_in stAddr = { 0 };
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = m_poConnection->GetRemoteIP();
	stAddr.sin_port = htons(m_poConnection->GetRemotePort());

	SetState(SSTATE_CONNECT);

#ifdef WIN32
	// todo connectEX暂时不能用 所以暂时不加到完成端口中//
#else
	if (!AddEvent())
	{
//		m_pLock.UnLock();
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return INVALID_SOCKET;
	}
#endif // WIN32

	//请求连接时 Windows跟linux是有区别的//
#ifdef WIN32
	//LPFN_CONNECTEX m_lpfnConnectEx = NULL ;
	//DWORD dwBytes = 0;
	//GUID GuidConnectEx = WSAID_CONNECTEX;

	//if (SOCKET_ERROR == WSAIoctl(GetSock(), SIO_GET_EXTENSION_FUNCTION_POINTER,
	//	&GuidConnectEx, sizeof (GuidConnectEx),
	//	&m_lpfnConnectEx, sizeof (m_lpfnConnectEx), &dwBytes, 0, 0))
	//{
	//	PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
	//	closesocket(GetSock());
	//	return INVALID_SOCKET;
	//}

	//// 这个时候 还没有连上 所以 将这个改成IOCP_CONNECT 等连完以后再改回来
	//m_stRecvIoData.nOp = IOCP_CONNECT;

	//int bResult = m_lpfnConnectEx(GetSock(),
	//	(sockaddr *)&stAddr,  // [in] 对方地址
	//	sizeof(stAddr),               // [in] 对方地址长度
	//	NULL,       // [in] 连接后要发送的内容，这里不用
	//	0,   // [in] 发送内容的字节数 ，这里不用
	//	NULL,       // [out] 发送了多少个字节，这里不用
	//	&m_stRecvIoData.stOverlapped); // [in]
	//if (!bResult)      // 返回值处//

	if(-1 == connect(GetSock(), (sockaddr*)&stAddr, sizeof(stAddr)))
	{
		// todo 直接阻塞住往上连
		PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
		closesocket(GetSock());
		return INVALID_SOCKET;
	}
	else
	{
		m_stRecvIoData.nOp = IOCP_RECV;
		unsigned long ul = 1;
		if (SOCKET_ERROR == ioctlsocket(GetSock(), FIONBIO, (unsigned long*)&ul))
		{
			PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
			closesocket(GetSock());
			LogScreen("Set socket FIONBIO error : %d", WSAGetLastError());
			return INVALID_SOCKET;
		}
		if (!AddEvent())
		{
			PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
			closesocket(GetSock());
			return INVALID_SOCKET;
		}
		OnConnect();
	}
#else
	if (-1 == connect(GetSock(), (sockaddr*)&stAddr, sizeof(stAddr)))
	{
		if (errno != EINPROGRESS && errno!= EINTR && errno != EAGAIN)
		{
//			m_pLock.UnLock();
			PushNetEvent(NETEVT_CONN_ERR, errno);
			LogScreen("connect error id : %d", errno);
			Close();
			return INVALID_SOCKET;
		}
	}
#endif // WIN32

	return GetSock();
}

void FxConnectSock::ProcEvent()
{
	SNetEvent* pEvent = m_oEvtQueue.PopFront();
	if (pEvent)
	{
		switch (pEvent->eType)
		{
		case NETEVT_ESTABLISH:
		{
			__ProcEstablish();
		}
			break;

		case NETEVT_ASSOCIATE:
		{
			__ProcAssociate();
		}
			break;
		case NETEVT_CONN_ERR:
		{
			__ProcConnectError(pEvent->dwValue);
		}
			break;

		case NETEVT_ERROR:
		{
			__ProcError(pEvent->dwValue);
		}
			break;

		case NETEVT_TERMINATE:
		{
			__ProcTerminate();
		}
			break;

		case NETEVT_RECV:
		{
			__ProcRecv(pEvent->dwValue);
		}
			break;

		case NETEVT_RELEASE:
		{
							   __ProcRelease();
		}
			break;

		default:
		{
				   assert(0);
		}
			break;
		}
	}
}

void FxConnectSock::OnConnect()
{
#ifdef WIN32
	sockaddr_in stAddr = {0};
	INT32 nAddrLen = sizeof(stAddr);
	getsockname(GetSock(), (sockaddr*)&stAddr, &nAddrLen);

	GetConnection()->SetLocalIP(stAddr.sin_addr.s_addr);
	GetConnection()->SetLocalPort(ntohs(stAddr.sin_port));

	GetConnection()->SetID(GetSockId());

	SetState(SSTATE_ESTABLISH);
	PushNetEvent(NETEVT_ESTABLISH, 0);

	if (false == PostRecv())
	{
		LogScreen("%s", "false == PostRecv()");
		LogFile("%s", "false == PostRecv()");
		LogFile(PrintTrace());
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		Close();
	}
#else
	SetState(SSTATE_ESTABLISH);
	PushNetEvent(NETEVT_ESTABLISH, 0);

	INT32 nError = 0;
	socklen_t nLen = sizeof(nError);
	if (getsockopt(GetSock(), SOL_SOCKET, SO_ERROR, &nError, &nLen) < 0)
	{
		LogScreen("getsockopt errno : %d", errno);
		LogFile("getsockopt errno : %d", errno);
		LogFile(PrintTrace());
		PushNetEvent(NETEVT_CONN_ERR, errno);
		Close();
		return;
	}

	if (nError != 0)
	{
		LogScreen("getsockopt errno : %d", nError);
		LogFile("getsockopt errno : %d", nError);
		LogFile(PrintTrace());
		PushNetEvent(NETEVT_CONN_ERR, nError);
		Close();
		return;
	}

	sockaddr_in stAddr = { 0 };
	UINT32 dwAddrLen = sizeof(stAddr);
	getsockname(GetSock(), (sockaddr*)&stAddr, &dwAddrLen);

	GetConnection()->SetLocalIP(stAddr.sin_addr.s_addr);
	GetConnection()->SetLocalPort(ntohs(stAddr.sin_port));

	GetConnection()->SetID(GetSockId());

	if (!m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
	{
		LogScreen("%s", "m_poIoThreadHandler->ChangeEvent");
		LogFile("%s", "m_poIoThreadHandler->ChangeEvent");
		LogFile(PrintTrace());
		PushNetEvent(NETEVT_CONN_ERR, errno);
		Close();
	}
#endif // WIN32

}

#ifdef WIN32
void FxConnectSock::OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred)
{
	if (NULL == pIoData)
	{
		Close();
		return;
	}

	switch (pIoData->nOp)
	{
	case IOCP_RECV:
	{
					  OnRecv(bRet, dwByteTransferred);
	}
		break;
	case IOCP_SEND:
	{
					  OnSend(bRet, dwByteTransferred);
	}
		break;
	case IOCP_CONNECT:
	{
						 if (GetState() != SSTATE_CONNECT)
						 {
							 assert(0);
						 }
						 OnConnect();
	}
		break;
	default:
	{
			   Close();
	}
		break;
	}
}

void FxConnectSock::OnRecv(bool bRet, int dwBytes)
{
	if (0 == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		Close();
		return;
	}

	if (UINT32(-1) == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostRecv, m_nPostRecv - 1, m_nPostRecv);
		if (0 == m_nPostRecv)
		{
			if (false == PostRecv())
			{
				PushNetEvent(NETEVT_ERROR, WSAGetLastError());
				Close();
			}
		}
		return;
	}

	if (false == bRet)
	{
		LogScreen("false == bRet errno : %d", WSAGetLastError());
		LogFile("false == bRet errno : %d", WSAGetLastError());
		LogFile(PrintTrace());
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		m_dwLastError = WSAGetLastError();
		PostClose();
		return;
	}

	if (!IsConnect())
	{
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		return;
	}

	m_oLock.Lock();
	int nUsedLen = 0;
	int nParserLen = 0;
	int nLen = int(dwBytes);
	if (m_poRecvBuf->CostBuff(nLen))
	{
		LogScreen("m_poRecvBuf->CostBuff error");
		LogFile("m_poRecvBuf->CostBuff error");
		LogFile(PrintTrace());
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		m_dwLastError = NET_RECVBUFF_ERROR;
		PostClose();
		return;
	}

	char *pUseBuf = NULL;
	nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
	if (nLen <= 0)
	{
		LogScreen("nLen <= 0");
		LogFile("nLen <= 0");
		LogFile(PrintTrace());
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		m_dwLastError = NET_RECVBUFF_ERROR;
		PostClose();
		m_oLock.UnLock();
		return;
	}

	while (0 < nLen)
	{
		if (0 != m_nNeedData)
		{
			if (nLen >= m_nNeedData)
			{
				nLen -= m_nNeedData;
				nUsedLen += m_nNeedData;
				nParserLen += m_nNeedData;
				m_poRecvBuf->CostUsedBuff(nUsedLen);
				nUsedLen = 0;
				PushNetEvent(NETEVT_RECV, m_nPacketLen);
				m_nPacketLen = 0;
				m_nNeedData = 0;
			}
			else
			{
				m_nNeedData -= nLen;
				nUsedLen += nLen;
				nLen = 0;
			}
		}
		else
		{
			char* pParseBuf = pUseBuf + nParserLen;
			UINT32 dwHeaderLen = GetDataHeader()->GetHeaderLength();
			GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen> nLen ? nLen : dwHeaderLen, 0);
			m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
			if (-1 == m_nPacketLen)
			{
				LogScreen("%s", "header error");
				LogFile("%s", "header error");
				LogFile(PrintTrace());
				InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
				m_dwLastError = NET_RECVBUFF_ERROR;
				PostClose();
				m_oLock.UnLock();
				return;
			}
			else if (0 == m_nPacketLen)
			{
				m_poRecvBuf->CostUsedBuff(nUsedLen);
				nUsedLen = 0;
				m_nNeedData = 0;

				if ((int)(GetDataHeader()->GetHeaderLength()) > nLen)
				{
					// 判断是否在循环buff的头部还有数据//
					int nHasData = m_poRecvBuf->GetUseLen();
					if ((int)(GetDataHeader()->GetHeaderLength()) <= nHasData)
					{
						GetDataHeader()->BuildRecvPkgHeader(pParseBuf, nLen, 0);
						if (false == m_poRecvBuf->CostUsedBuff(nLen))
						{
							break;
						}
						int nNewLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
						if ((int)(GetDataHeader()->GetHeaderLength()) - nLen > nNewLen)
						{
							LogScreen("%s", "header error");
							LogFile("%s", "header error");
							LogFile(PrintTrace());
							InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
							m_dwLastError = NET_RECVBUFF_ERROR;
							PostClose();
							m_oLock.UnLock();
							return;
						}
						GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
						pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
						m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
						if (0 >= m_nPacketLen)
						{
							LogScreen("%s", "header error");
							LogFile("%s", "header error");
							LogFile(PrintTrace());
							InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
							m_dwLastError = NET_RECVBUFF_ERROR;
							PostClose();
							m_oLock.UnLock();
							return;
						}

						m_nNeedData = m_nPacketLen - GetDataHeader()->GetHeaderLength();
						nUsedLen = GetDataHeader()->GetHeaderLength() - nLen;
						nParserLen = nUsedLen;
						nLen = nNewLen - nUsedLen;
						pParseBuf = pUseBuf + nUsedLen;
						if (0 == m_nNeedData)
						{
							m_poRecvBuf->CostUsedBuff(nUsedLen);
							nUsedLen = 0;
							PushNetEvent(NETEVT_RECV, m_nPacketLen);
							m_nPacketLen = 0;
						}
					}
					else
					{
						nLen = 0;
						break;
					}
				}
			}
			else
			{
				if (nLen >= m_nPacketLen)
				{
					nLen -= m_nPacketLen;
					nUsedLen += m_nPacketLen;
					nParserLen += nUsedLen;
					m_poRecvBuf->CostUsedBuff(nUsedLen);
					nUsedLen = 0;
					PushNetEvent(NETEVT_RECV, m_nPacketLen);
					m_nPacketLen = 0;
					m_nNeedData = 0;
				}
				else
				{
					m_nNeedData = m_nPacketLen - nLen;
					nUsedLen += nLen;
					nLen = 0;
				}
			}
		}
	}

	if (0 != nUsedLen)
	{
		m_poRecvBuf->CostUsedBuff(nUsedLen);
	}

	InterlockedCompareExchange(&m_nPostRecv, m_nPostRecv - 1, m_nPostRecv);
	if (0 == m_nPostRecv)
	{
		if (false == PostRecv())
		{
			LogScreen("%s", "false == PostRecv");
			LogFile("%s", "false == PostRecv");
			LogFile(PrintTrace());
			m_dwLastError = WSAGetLastError();
			PostClose();
		}
	}
	m_oLock.UnLock();
}

void FxConnectSock::OnSend(bool bRet, int dwBytes)
{
	if (false == bRet)
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		m_dwLastError = WSAGetLastError();
		PostClose();
		return;
	}

	if (false == IsConnect())
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		return;
	}

	if (SSTATE_ESTABLISH != GetState())
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		return;
	}

	if (UINT32(-1) == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostSend, m_nPostSend - 1, m_nPostSend);
		if (0 == m_nPostSend)
		{
			if (false == PostSend())
			{
				LogScreen("%s", "false == PostSend");
				LogFile("%s", "false == PostSend");
				LogFile(PrintTrace());
				m_dwLastError = WSAGetLastError();
				PostClose();
			}
		}
		return;
	}

	if (0 == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		m_dwLastError = NET_SEND_ERROR;
		PostClose();
		return;
	}

	// 把成功发送了的从发送缓冲区丢弃。//
	m_poSendBuf->DiscardBuff(dwBytes);

	InterlockedCompareExchange(&m_nPostSend, m_nPostSend - 1, m_nPostSend);
	if (0 == m_nPostSend)
	{
		if (false == PostSend())
		{
			LogScreen("%s", "false == PostSend");
			LogFile("%s", "false == PostSend");
			LogFile(PrintTrace());
			m_dwLastError = WSAGetLastError();
			PostClose();
		}
	}
}

bool FxConnectSock::PostRecv()
{
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return false;
	}

	LONG nPostRecv = InterlockedCompareExchange(&m_nPostRecv, 1, 0);
	if (0 != nPostRecv)
	{
		return true;
	}

	if (m_poRecvBuf->IsEmpty())
	{
		m_poRecvBuf->Clear();
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	int nLen = m_poRecvBuf->GetInCursorPtr(m_stRecvIoData.stWsaBuf.buf);
	if (0 >= nLen)
	{
		LogScreen("%s", "0 >= nLen");
		LogFile("%s", "0 >= nLen");
		LogFile(PrintTrace());
		// 这时候 肯定出问题了//
		InterlockedCompareExchange(&m_nPostRecv, 0, 1);
		PostClose();
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;
	m_stRecvIoData.stWsaBuf.len = nLen;

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	if (0 != WSARecv(GetSock(), &m_stRecvIoData.stWsaBuf, 1, &dwReadLen, &dwFlags, &m_stRecvIoData.stOverlapped, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogScreen("WSARecv errno : %d", WSAGetLastError());
			LogFile("WSARecv errno : %d", WSAGetLastError());
			LogFile(PrintTrace());
			InterlockedCompareExchange(&m_nPostRecv, 0, 1);
			return false;
		}
	}

	return true;
}

bool FxConnectSock::PostClose()
{
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return false;
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	// Post失败的时候再进入这个函数时可能会丢失一次。//

	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), UINT32(0), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		LogScreen("PostQueuedCompletionStatus errno : %d", WSAGetLastError());
		LogFile("PostQueuedCompletionStatus errno : %d", WSAGetLastError());
		LogFile(PrintTrace());
		return false;
	}

	return true;
}

#else
void FxConnectSock::OnParserIoEvent(int dwEvents)
{
	if(GetState() == SSTATE_CONNECT)
	{
		if(dwEvents & EPOLLOUT)
		{
			OnConnect();
		}
		else
		{
			PushNetEvent(NETEVT_ERROR, errno);
			Close();
		}
		return;
	}
	if (!IsConnect())
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return;
	}

	if (dwEvents & EPOLLOUT)
	{
		OnSend();
	}

	if (dwEvents & EPOLLIN)
	{
		OnRecv();
	}

	if (dwEvents & EPOLLERR)
	{
		if(errno == EINPROGRESS || errno == EINTR || errno == EAGAIN)
		{
			return;
		}
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
	}
}

void FxConnectSock::OnRecv()
{
	if (false == IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return;
	}

	if (m_poRecvBuf->IsEmpty())
	{
		m_poRecvBuf->Clear();
	}

	char* pRecvBuff = NULL;
	int nLen = m_poRecvBuf->GetInCursorPtr(pRecvBuff);
	if (0 >= nLen)
	{
		return;
	}

	nLen = VAL_SO_SNDLOWAT < nLen ? VAL_SO_SNDLOWAT : nLen;

	nLen = recv(GetSock(), pRecvBuff, nLen, 0);

	if(0 > nLen)
	{
		if ((errno != EAGAIN) && (errno != EINPROGRESS) && (errno != EINTR))
		{
			LogScreen("recv errno : %d", errno);
			LogFile("recv errno : %d", errno);
			LogFile(PrintTrace());
			PushNetEvent(NETEVT_ERROR, errno);
			Close();
			return;
		}
		return;
	}
	else if (0 == nLen)
	{
		Close();
		return;
	}
	else
	{
		int nUsedLen = 0;
		int nParserLen = 0;
		m_poRecvBuf->CostBuff(nLen);

		char *pUseBuf = NULL;
		nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
		if (nLen <= 0)
		{
			LogScreen("nLen <= 0");
			LogFile("nLen <= 0");
			LogFile(PrintTrace());
			PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
			Close();
			return;
		}


		while (0 < nLen)
		{
			if (0 != m_nNeedData)
			{
				if (nLen >= m_nNeedData)
				{
					nLen -= m_nNeedData;
					nUsedLen += m_nNeedData;
					nParserLen += m_nNeedData;
					m_poRecvBuf->CostUsedBuff(nUsedLen);
					nUsedLen = 0;
					PushNetEvent(NETEVT_RECV, m_nPacketLen);
					m_nPacketLen = 0;
					m_nNeedData = 0;
				}
				else
				{
					m_nNeedData -= nLen;
					nUsedLen += nLen;
					nLen = 0;
				}
			}
			else
			{
				char* pParseBuf = pUseBuf + nParserLen;
				UINT32 dwHeaderLen = GetDataHeader()->GetHeaderLength();
				GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen> nLen ? nLen : dwHeaderLen, 0);
				m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
				if (-1 == m_nPacketLen)
				{
					LogScreen("%s", "header error");
					LogFile("%s", "header error");
					LogFile(PrintTrace());
					PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
					Close();
					return;
				}
				else if (0 == m_nPacketLen)
				{
					m_poRecvBuf->CostUsedBuff(nUsedLen);
					nUsedLen = 0;
					m_nNeedData = 0;

					if ((int)(GetDataHeader()->GetHeaderLength()) > nLen)
					{
							// 判断是否在循环buff的头部还有数据//
						int nHasData = m_poRecvBuf->GetUseLen();
						if ((int)(GetDataHeader()->GetHeaderLength()) <= nHasData)
						{
							GetDataHeader()->BuildRecvPkgHeader(pParseBuf, nLen, 0);
							if (false == m_poRecvBuf->CostUsedBuff(nLen))
							{
								break;
							}
							int nNewLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
							if ((int)(GetDataHeader()->GetHeaderLength()) - nLen > nNewLen)
							{
								LogScreen("%s", "header error");
								LogFile("%s", "header error");
								LogFile(PrintTrace());
								PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
								Close();
								return;
							}
							GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
							pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
							m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
							if (0 >= m_nPacketLen)
							{
								LogScreen("%s", "header error");
								LogFile("%s", "header error");
								LogFile(PrintTrace());
								PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
								Close();
								return;
							}

							m_nNeedData = m_nPacketLen - GetDataHeader()->GetHeaderLength();
							nUsedLen = GetDataHeader()->GetHeaderLength() - nLen;
							nParserLen = nUsedLen;
							nLen = nNewLen - nUsedLen;
							pParseBuf = pUseBuf + nUsedLen;
							if (0 == m_nNeedData)
							{
								m_poRecvBuf->CostUsedBuff(nUsedLen);
								nUsedLen = 0;
								PushNetEvent(NETEVT_RECV, m_nPacketLen);
								m_nPacketLen = 0;
							}
						}
						else
						{
							nLen = 0;
							break;
						}
					}
				}
				else
				{
					if (nLen >= m_nPacketLen)
					{
						nLen -= m_nPacketLen;
						nUsedLen += m_nPacketLen;
						nParserLen += nUsedLen;
						m_poRecvBuf->CostUsedBuff(nUsedLen);
						nUsedLen = 0;
						PushNetEvent(NETEVT_RECV, m_nPacketLen);
						m_nPacketLen = 0;
						m_nNeedData = 0;
					}
					else
					{
						m_nNeedData = m_nPacketLen - nLen;
						nUsedLen += nLen;
						nLen = 0;
					}
				}
			}
		}

		if (0 != nUsedLen)
		{
			m_poRecvBuf->CostUsedBuff(nUsedLen);
		}
	}
}

void FxConnectSock::OnSend()
{
	if (!IsConnect())
	{
		LogScreen("%s", "false == IsConnect()");
		LogFile("%s", "false == IsConnect()");
		LogFile(PrintTrace());
		return;
	}

	m_bSending = false;
	if (!PostSend())
	{
		LogScreen("%s", "false == PostSend()");
		LogFile("%s", "false == PostSend()");
		LogFile(PrintTrace());
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return;
	}
}


#endif // WIN32


