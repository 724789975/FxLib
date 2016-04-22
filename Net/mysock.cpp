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
#else
		LogScreen("create socket error, %u:%u, errno %d", dwIP, wPort, errno);
#endif // WIN32
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
#else
		LogScreen("bind at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32

		return false;
	}
	if (listen(GetSock(), 128) < 0)
	{
#ifdef WIN32
		LogScreen("listen at %u:%d failed, errno %d", dwIP, wPort, WSAGetLastError());
#else
		LogScreen("listen at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32
		return false;
	}

	m_poEpollHandler = FxNetModule::Instance()->FetchIoThread(GetSockId());
	if (NULL == m_poEpollHandler)
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
		LogScreen("CCpListener::Start, _InitAcceptEx failed");
		return false;
	}

	for (INT32 i = 0; i < sizeof(m_oSPerIoDatas) / sizeof(m_oSPerIoDatas[0]); i++)
	{
		if (false == PostAccept(m_oSPerIoDatas[i]))
		{
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
		return false;
	}

	if (INVALID_SOCKET == GetSock())
	{
		return false;
	}

#ifdef WIN32
	CancelIo((HANDLE)GetSock());
	closesocket(GetSock());
	SetSock(INVALID_SOCKET);
#else

	m_poEpollHandler->DelEvent(GetSock());
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
	m_poEpollHandler->DelEvent(GetSock());
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
//	m_pLock.Lock();
	if (SSTATE_INVALID == GetState())
	{
//		m_pLock.UnLock();
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

//	while (!FxNetModule::Instance()->PushNetEvent(this))
//	{
//		FxSleep(1);
//	}

	while(!m_poEpollHandler->PushSock(this))
	{
		break;
		FxSleep(1);
	}

//		m_pLock.UnLock();
	return true;
}

bool FxListenSock::AddEvent()
{
#ifdef WIN32
	if (!m_poEpollHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return false;
	}
#else
	if (!m_poEpollHandler->AddEvent(GetSock(), EPOLLIN, this))
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
		return false;
	}

	unsigned long ul = 1;
	if (SOCKET_ERROR == ioctlsocket(hNewSock, FIONBIO, (unsigned long*)&ul))
	{
		closesocket(hNewSock);
		LogScreen("Set socket FIONBIO error : %d", WSAGetLastError());
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
			LogScreen("CCpListener::PostAcceptEx, AcceptEx failed, errno %d", WSAGetLastError());
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
			closesocket(pIoData->hSock);
			PostAccept(*pIoData);
		}
		OnAccept(pIoData);
		m_oLock.UnLock();
	}
	else
	{
		Close();        // 未知错误，不应该发生//
	}
}

void FxListenSock::OnAccept(SPerIoData* pstPerIoData)
{
	SOCKET hSock = pstPerIoData->hSock;

	if (SSTATE_LISTEN != GetState())
	{
		closesocket(hSock);
		return;
	}

	{
		FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
		if (NULL == poSock)
		{
			LogScreen("CCPSock::OnAccept, create CCPSock failed");
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxIoThread* poEpollHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSockId());
		if (NULL == poEpollHandler)
		{
			LogScreen("CCPSock::OnAccept, get iothread failed");
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
		if (NULL == poConnection)
		{
			LogScreen("CCPSock::OnAccept, create CSDConnection failed");
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
			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		poSession->Init(poConnection);
		poConnection->SetSession(poSession);
		poSock->SetIoThread(poEpollHandler);

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
			LogScreen("Set keep alive error: %d\n", WSAGetLastError());
			PostAccept(*pstPerIoData);
			poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
			poSock->Close();
			return;
		}

		//poSock->SetConnect(true);

		//
		// 应该先投递连接事件再关联套接口，否则可能出现第一个Recv事件先于连接事件入队列
		//

		if (false == poSock->AddEvent())
		{
			poSock->Close();
		}
		else
		{
			poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

			if (false == poSock->PostRecv())
			{
				poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
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
		return;
	}

	FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
	if (NULL == poSock)
	{
		close(hAcceptSock);
		return;
	}

	FxIoThread* poEpollHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSockId());
	if (NULL == poEpollHandler)
	{
		close(hAcceptSock);
		return;
	}

	FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
	if (NULL == poConnection)
	{
		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		return;
	}

	FxSession* poSession = m_poSessionFactory->CreateSession();
	if (NULL == poSession)
	{
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

//	LogScreen("total %d socket accept", ++nAccept);
	if(!poSock->AddEvent())
	{
//		LogScreen("sock %d", hAcceptSock);
		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
//		FxConnectionMgr::Instance()->Release(poConnection);

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
		//FxConnectionMgr::Instance()->Release(m_poConnection);
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
		return false;
	}

	if (NULL == m_poSendBuf)
	{
		m_poSendBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poSendBuf)
		{
			return false;
		}
	}

	if (NULL == m_poRecvBuf)
	{
		m_poRecvBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poRecvBuf)
		{
			return false;
		}
	}

	if (!m_poRecvBuf->Init(RECV_BUFF_SIZE))
	{
		return false;
	}

	if (!m_poSendBuf->Init(SEND_BUFF_SIZE))
	{
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

int nClose = 0;
int nCloseBefore = 0;
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

//	LogScreen("total %d close" , ++nClose);
	if (IsConnect())
	{
		SetState(SSTATE_CLOSE);
	}
	if (GetSock() == INVALID_SOCKET)
	{
		m_oLock.UnLock();
		return true;
	}
//	if(m_poConnection)
//	{
//		m_poConnection->OnSocketDestroy();
//	}
//	LogScreen("socket id : %d handler : %d addr : %d, sock %d", GetSockId(), m_poEpollHandler, this, GetSock());

#ifdef WIN32
	shutdown(GetSock(), SD_RECEIVE);
#else
	shutdown(GetSock(), SHUT_RD);
	m_poEpollHandler->DelEvent(GetSock());
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

//	LogScreen("close before event %d", ++nCloseBefore);
	PushNetEvent(NETEVT_TERMINATE, 0);
	m_oLock.UnLock();

	return true;
}

void FxConnectSock::Reset()
{
	if (NULL != m_poConnection)
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		m_poConnection->OnSocketDestroy();
		// 让session去负责release吧 只要session存在 那么就肯定有connection//
		//FxConnectionMgr::Instance()->Release(m_poConnection);
		//m_poConnection = NULL;
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

//	if (m_pLock)
//	{
//		m_pLock.Release();
//		m_pLock = NULL;
//	}

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
	m_oLock.Lock();
	if (false == IsConnect())
	{
		m_oLock.UnLock();
		return false;
	}

	if (GetState() != SSTATE_ESTABLISH)
	{
		m_oLock.UnLock();
		return false;
	}

	if (m_poSendBuf->IsEmpty())
	{
		m_poSendBuf->Clear();
	}

	if (dwLen > m_poSendBuf->GetTotalLen())
	{
		m_oLock.UnLock();
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		Close();
#endif // WIN32
		return false;
	}

	int nSendCount = 0;
	//SPacketHeader oHeader = { 0 };
	//int nHeaderLen = m_oPacketParser.BuildSendPkgHeader(&oHeader, dwLen);
	IFxDataHeader* pDataHeader = GetDataHeader();
	if (pDataHeader == NULL)
	{
		m_oLock.UnLock();
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		Close();
#endif // WIN32
		return false;
	}
	while (!m_poSendBuf->PushBuff((char*)(pDataHeader->BuildSendPkgHeader(dwLen)), pDataHeader->GetHeaderLength()))
	{
		m_oLock.UnLock();
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			return false;
		}
		FxSleep(1);
		m_oLock.Lock();
	}

	while (!m_poSendBuf->PushBuff(pData, dwLen))
	{
		m_oLock.UnLock();
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			return false;
		}
		FxSleep(1);
		m_oLock.Lock();
	}

	if (false == PostSendFree())
	{
		m_oLock.UnLock();
#ifdef WIN32
		m_dwLastError = WSAGetLastError();
		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		Close();
#endif // WIN32
		return false;
	}

	m_oLock.UnLock();
	return true;
}

bool FxConnectSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{
	// 首先 状态要对 状态不对的话 抛不进事件的//
//	m_pLock.Lock();
//	if (GetState() == SSTATE_INVALID)
//	{
//		m_pLock.UnLock();
//		return false;
//	}
	{
		if(eType == NETEVT_ERROR || eType == NETEVT_TERMINATE)
		{
		}
	}
	SNetEvent oEvent;
	// 先扔网络事件进去，然后在报告上层有事件，先后顺序不能错，这样上层就不会错取事件//
	oEvent.eType = eType;
	oEvent.dwValue = dwValue;

//	if(GetState() == SSTATE_CLOSE)
//	{
//		if(eType != NETEVT_TERMINATE && eType != NETEVT_RELEASE)
//		{
//			LogScreen("socket close error");
//			m_pLock.UnLock();
//			return false;
//		}
//	}
//		LogScreen("socket id %d, addr %d, event type %d, value %d, sock %d, state %d, thread id %d", GetSockId(), this, eType, dwValue, GetSock(), GetState(), pthread_self());
	while (!m_oEvtQueue.PushBack(oEvent))
	{
		FxSleep(1);
	}

//	while (!FxNetModule::Instance()->PushNetEvent(this))
//	{
//		FxSleep(1);
//	}

//	while(!m_poEpollHandler->PushSock(this))
//	{
//		break;
//		FxSleep(1);
//	}

	if(!m_poEpollHandler->PushSock(this))
	{
		while (!FxNetModule::Instance()->PushNetEvent(this))
		{
			FxSleep(1);
		}
	}

//	m_pLock.UnLock();
	return true;
}

bool FxConnectSock::PostSend()
{
#ifdef WIN32
	if (false == IsConnect())
	{
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
			return false;
		}
	}

	return true;
#else
	if (false == IsConnect())
	{
		return false;
	}

	if (NULL == m_poEpollHandler)
	{
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
		if (false == m_poEpollHandler->ChangeEvent(GetSock(), EPOLLIN, this))
		{
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
			if (false == m_poEpollHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
			{
				m_bSending = false;
				return false;
			}
			return true;
		}

		m_bSending = false;
		return false;
	}
	else if (0 == nRet)
	{
		m_bSending = false;
		return false;
	}

	// 把成功发送了的从发送缓冲区丢弃。//
	m_poSendBuf->DiscardBuff(nRet);

	if (false == m_poEpollHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
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
		return false;
	}

	if (NULL == m_poEpollHandler)
	{
		Close();
		return false;
	}

	return m_poEpollHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this);
//	if (!m_bSending)    //
//	{
//		return PostSend();
//	}

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

	if (NULL == m_poEpollHandler)
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
//			LogScreen("socket id : %d, connect id : %d", GetSockId(), m_poConnection->GetID());
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
//			LogScreen("socket id : %d, connect id : %d", GetSockId(), m_poConnection->GetID());
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
//			LogScreen("socket id : %d, connect id : %d", GetSockId(), m_poConnection->GetID());
			return;
		}

		m_poConnection->OnError(dwErrorNo);
	}
}

int nTerminate = 0;
void FxConnectSock::__ProcTerminate()
{
//	LogScreen("total %d terminate", ++nTerminate);
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
//			LogScreen("socket id : %d, connect id : %d", GetSockId(), m_poConnection->GetID());
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

int nRelease1 = 0;
void FxConnectSock::__ProcRelease()
{
//	LogScreen("total %d release", ++nRelease1);
	if (m_poConnection)
	{
		if (GetSockId() != m_poConnection->GetID())
		{
//			LogScreen("socket id : %d, connect id : %d", GetSockId(), m_poConnection->GetID());
			return;
		}

		m_poConnection->OnSocketDestroy();
		m_poConnection->OnClose();

		SetConnection(NULL);
	}
//	LogScreen("socket id %d, addr %d, sock %d", GetSockId(), this, GetSock());
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
	if (!m_poEpollHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		LogScreen("error : %d", WSAGetLastError());
		Close();
		return false;
	}
#else
	if (!m_poEpollHandler->AddEvent(GetSock(), EPOLLOUT|EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		LogScreen("error : %d", errno);
		Close();
		return false;
	}
#endif // WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;

}

SOCKET FxConnectSock::Connect()
{
//	m_pLock.Lock();
	if (IsConnect())
	{
//		m_pLock.UnLock();
		return INVALID_SOCKET;
	}

	if (NULL == m_poConnection)
	{
//		m_pLock.UnLock();
		return INVALID_SOCKET;
	}

	if (m_poConnection->IsConnected())
	{
//		m_pLock.UnLock();
		return INVALID_SOCKET;
	}

	if (INVALID_SOCKET != GetSock())
	{
//		m_pLock.UnLock();
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
//		m_pLock.UnLock();
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
	// keep alive windows下怎么加?  暂时先不添加了//
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
	if (NULL == m_poEpollHandler)
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
	LPFN_CONNECTEX m_lpfnConnectEx = NULL ;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	if (SOCKET_ERROR == WSAIoctl(GetSock(), SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx, sizeof (GuidConnectEx),
		&m_lpfnConnectEx, sizeof (m_lpfnConnectEx), &dwBytes, 0, 0))
	{
		PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
		closesocket(GetSock());
		return INVALID_SOCKET;
	}

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
		//if(WSAEWOULDBLOCK != WSAGetLastError())
		//{
		//	PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
		//	closesocket(GetSock());
		//	return INVALID_SOCKET;
		//}
		//else;//
		//{
		//}
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
			return false;
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

//	m_pLock.UnLock();
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
		PushNetEvent(NETEVT_CONN_ERR, errno);
		Close();
		return;
	}

	if (nError != 0)
	{
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

	if (!m_poEpollHandler->ChangeEvent(GetSock(), EPOLLIN, this))
	{
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
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		Close();
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
	m_poRecvBuf->CostBuff(nLen);

	char *pUseBuf = NULL;
	nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
	if (nLen <= 0)
	{
		PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
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
				PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
				InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
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
							PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
							InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
							PostClose();
							m_oLock.UnLock();
							return;
						}
						GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
						pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
						m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
						if (0 >= m_nPacketLen)
						{
							PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
							InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
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
			PushNetEvent(NETEVT_ERROR, WSAGetLastError());
			PostClose();
		}
	}
	m_oLock.UnLock();
}

void FxConnectSock::OnSend(bool bRet, int dwBytes)
{
	//UINT32 dwPort = GetConnection()->GetRemotePort();
	if (false == bRet)
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		Close();
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
				PushNetEvent(NETEVT_ERROR, WSAGetLastError());
				Close();
			}
		}
		return;
	}

	if (0 == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		Close();
		return;
	}

	// 把成功发送了的从发送缓冲区丢弃。//
	m_poSendBuf->DiscardBuff(dwBytes);

	InterlockedCompareExchange(&m_nPostSend, m_nPostSend - 1, m_nPostSend);
	if (0 == m_nPostSend)
	{
		if (false == PostSend())
		{
			PushNetEvent(NETEVT_ERROR, WSAGetLastError());
			Close();
		}
	}
}

bool FxConnectSock::PostRecv()
{
	if (false == IsConnect())
	{
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
		return false;
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	// Post失败的时候再进入这个函数时可能会丢失一次。//

	if (!PostQueuedCompletionStatus(m_poEpollHandler->GetHandle(), UINT32(0), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
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
								PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
								Close();
								return;
							}
							GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
							pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
							m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
							if (0 >= m_nPacketLen)
							{
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
		return;
	}

	m_bSending = false;
	if (!PostSend())
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return;
	}
}


#endif // WIN32


