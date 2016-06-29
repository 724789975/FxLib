#include "myudpsock.h"
#include "sockmgr.h"
#include <stdio.h>
#include "connectionmgr.h"
#include "connection.h"
#include "iothread.h"
#include "net.h"
#include "netstream.h"

#define RECV_BUFF_SIZE 8*64*1024
#define SEND_BUFF_SIZE 64*64*1024

FxUDPListenSock::FxUDPListenSock()
{
	Reset();

	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_poSessionFactory = NULL;
}

FxUDPListenSock::~FxUDPListenSock()
{
	if (m_poSessionFactory)
	{
		delete m_poSessionFactory;
		m_poSessionFactory = NULL;
	}
}

bool FxUDPListenSock::Init()
{
	return m_oEvtQueue.Init(MAX_NETEVENT_PERSOCK);
}

void FxUDPListenSock::OnRead()
{

}

void FxUDPListenSock::OnWrite()
{

}

bool FxUDPListenSock::Listen(UINT32 dwIP, UINT16 wPort)
{
	SetSock(socket(AF_INET, SOCK_DGRAM, 0));
	if (INVALID_SOCKET == GetSock())
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
		LogFun(LT_Screen | LT_File, LogLv_Error, "create socket error, %u:%u, errno %d", dwIP, wPort, dwErr);
#else
		LogFun(LT_Screen | LT_File, LogLv_Error, "create socket error, %u:%u, errno %d", dwIP, wPort, errno);
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

#ifdef WIN32
	unsigned long ul = 1;
	if (SOCKET_ERROR == ioctlsocket(GetSock(), FIONBIO, (unsigned long*)&ul))
	{
		PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
		closesocket(GetSock());
		LogFun(LT_Screen | LT_File, LogLv_Error, "Set socket FIONBIO error : %d, socket : %d, socket id %d",
			WSAGetLastError(), GetSock(), GetSockId());
		return false;
	}
#endif // WIN32

	if (bind(GetSock(), (sockaddr*)&stAddr, sizeof(stAddr)) < 0)
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
		LogFun(LT_Screen | LT_File, LogLv_Error, "bind at %u:%d failed, errno %d", dwIP, wPort, dwErr);
#else
		LogFun(LT_Screen | LT_File, LogLv_Error, "bind at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32
		return false;
	}

	m_poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(GetSockId());
	if (NULL == m_poIoThreadHandler)
	{
		Close();
		return false;
	}

	SetState(SSTATE_LISTEN);

	// 添加到事件 //
	if (false == AddEvent())
	{
		return false;
	}
	LogFun(LT_Screen | LT_File, LogLv_Info, "listen at %u:%d success", dwIP, wPort);

#ifdef WIN32
	for (int i = 0; i < sizeof(m_oSPerIoDatas) / sizeof(SPerUDPIoData); ++i)
	{
		m_oSPerIoDatas[i].stWsaBuf.buf = (char*)(&m_oPacketHeaders[i]);
		m_oSPerIoDatas[i].stWsaBuf.len = sizeof(m_oPacketHeaders[i]);
		PostAccept(m_oSPerIoDatas[i]);
	}
#endif // WIN32
	return true;
}

bool FxUDPListenSock::StopListen()
{
	if (SSTATE_LISTEN != GetState())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "state : %d != SSTATE_LISTEN", (UINT32)GetState());
		return false;
	}

	if (INVALID_SOCKET == GetSock())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "socket : %d == INVALID_SOCKET", GetSock());
		return false;
	}

#ifdef WIN32
	shutdown(GetSock(), SD_RECEIVE);
#else
	shutdown(GetSock(), SHUT_RD);
#endif // WIN32

	SetState(SSTATE_STOP_LISTEN);

	// 不close 如需要 另外close
	return true;
}

bool FxUDPListenSock::Close()
{
	m_oLock.Lock();
	if (GetState() == SSTATE_CLOSE)
	{
		m_oLock.UnLock();
		return true;
	}
	SetState(SSTATE_CLOSE);
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

void FxUDPListenSock::Reset()
{
	m_poSessionFactory = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
}

bool FxUDPListenSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{
	if (SSTATE_INVALID == GetState())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "state : %d == SSTATE_INVALID", (UINT32)GetState());

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

	while (!FxNetModule::Instance()->PushNetEvent(this))
	{
		FxSleep(1);
	}

	return true;
}

void FxUDPListenSock::ProcEvent()
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
			Assert(0);
		}
		break;
		}
	}
}

#ifdef WIN32
void FxUDPListenSock::OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred)
{
	SPerUDPIoData* pSPerUDPIoData = (SPerUDPIoData*)pIoData;
	switch (GetState())
	{
	case SSTATE_LISTEN:
	{
		m_oLock.Lock();
		if (false == bRet)
		{
			int dwErr = WSAGetLastError();
			LogFun(LT_Screen | LT_File, LogLv_Error, "OnParserIoEvent failed, errno %d", dwErr);

			closesocket(pSPerUDPIoData->hSock);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}
		if (dwByteTransferred != sizeof(UDPPacketHeader))
		{
			LogFun(LT_File | LT_Screen, LogLv_Error, "%s", "recv length error !!!");
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)pSPerUDPIoData->stWsaBuf.buf;

		if ((ESocketState)pUDPPacketHeader->m_cStatus != SSTATE_CONNECT)
		{
			LogFun(LT_File | LT_Screen, LogLv_Error, "recv socket state error want : %d, recv : %d !!!",
				SSTATE_CONNECT, pUDPPacketHeader->m_cStatus);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		if (pUDPPacketHeader->m_cSyn != 1)
		{
			LogFun(LT_File | LT_Screen, LogLv_Error, "recv socket syn error want : %d, recv : %d !!!",
				1, pUDPPacketHeader->m_cSyn);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}
		OnAccept(pSPerUDPIoData);
		m_oLock.UnLock();
	}
		break;
	case SSTATE_CLOSE:
	case SSTATE_STOP_LISTEN:
	{
		m_oLock.Lock();
		if (bRet)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "listen socket has stoped but ret is true");
		}
		else
		{
			pSPerUDPIoData->hSock = INVALID_SOCKET;
		}
		m_oLock.UnLock();
	}
		break;
	default:
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "state : %d != SSTATE_LISTEN", (UINT32)GetState());

		Close();        // 未知错误，不应该发生//
	}
		break;
	}
}
#else
void FxUDPListenSock::OnParserIoEvent(int dwEvents)
{

}
#endif // WIN32

bool FxUDPListenSock::AddEvent()
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

void FxUDPListenSock::__ProcAssociate()
{

}

void FxUDPListenSock::__ProcError(UINT32 dwErrorNo)
{

}

void FxUDPListenSock::__ProcTerminate()
{

}

#ifdef WIN32
bool FxUDPListenSock::PostAccept(SPerUDPIoData& oSPerIoData)
{
	LONG nPostRecv = InterlockedCompareExchange(&m_nPostRecv, 1, 0);
	if (0 != nPostRecv)
	{
		return true;
	}

	memset(&oSPerIoData.stOverlapped, 0, sizeof(oSPerIoData.stOverlapped));

	memset(oSPerIoData.stWsaBuf.buf, 0, oSPerIoData.stWsaBuf.len);

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	int dwSockAddr = sizeof(oSPerIoData.stRemoteAddr);

	if (WSARecvFrom(GetSock(), &oSPerIoData.stWsaBuf, 1, &dwReadLen, &dwFlags,
		(sockaddr*)(&oSPerIoData.stRemoteAddr), &dwSockAddr, &oSPerIoData.stOverlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "WSARecvFrom errno : %d, socket : %d, socket id : %d",
				WSAGetLastError(), GetSock(), GetSockId());

			InterlockedCompareExchange(&m_nPostRecv, 0, 1);
			return false;
		}
	}
	return true;
}

void FxUDPListenSock::OnAccept(SPerUDPIoData* pstPerIoData)
{
	SOCKET hSock = pstPerIoData->hSock;

	if (SSTATE_LISTEN != GetState())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "state : %d != SSTATE_LISTEN", GetState());

		closesocket(hSock);
		return;
	}

	{
		FxUDPConnectSock* poSock = FxMySockMgr::Instance()->CreateUdpSock();
		if (NULL == poSock)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "CCPSock::OnAccept, create CCPSock failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxIoThread* poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSockId());
		if (NULL == poIoThreadHandler)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "CCPSock::OnAccept, get iothread failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->ReleaseUdpSock(poSock);
			return;
		}

		FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
		if (NULL == poConnection)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "CCPSock::OnAccept, create Connection failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->ReleaseUdpSock(poSock);
			return;
		}

		poSock->SetSock(hSock);
		poSock->SetConnection(poConnection);

		poConnection->SetSockType(SOCKTYPE_TCP);
		poConnection->SetSock(poSock);
		poConnection->SetID(poSock->GetSockId());

		sockaddr_in stLocalAddr;
		INT32 nLocalAddrLen = sizeof(sockaddr_in);
		
		if (getsockname(GetSock(), (sockaddr*)(&stLocalAddr), &nLocalAddrLen) == SOCKET_ERROR)
		{
			int dwErr = WSAGetLastError();
			LogFun(LT_Screen | LT_File, LogLv_Error, "getsockname error: %d", dwErr);

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->ReleaseUdpSock(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		poConnection->SetLocalIP(stLocalAddr.sin_addr.s_addr);
		poConnection->SetLocalPort(ntohs(stLocalAddr.sin_port));

		poConnection->SetRemoteIP(pstPerIoData->stRemoteAddr.sin_addr.s_addr);
		poConnection->SetRemotePort(ntohs(pstPerIoData->stRemoteAddr.sin_port));

		FxSession* poSession = m_poSessionFactory->CreateSession();
		if (NULL == poSession)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "CCPSock::OnAccept, CreateSession failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->ReleaseUdpSock(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		poSession->Init(poConnection);
		poConnection->SetSession(poSession);
		poSock->SetIoThread(poIoThreadHandler);

		poSock->SetState(SSTATE_ESTABLISH);

		if (connect(poSock->GetSock(), (sockaddr*)(&pstPerIoData->stRemoteAddr), sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());
				
				PostAccept(*pstPerIoData);
				poSock->Close();
				return;
			}
		}
		// 这个时候不能说是已经establish 了 要发个消息确认下
		UDPPacketHeader oUDPPacketHeader;
		oUDPPacketHeader.m_cAck = 1;
		oUDPPacketHeader.m_cSyn = ((UDPPacketHeader*)(pstPerIoData->stWsaBuf.buf))->m_cSyn;
		oUDPPacketHeader.m_cStatus = poSock->GetState();

		// todo send的时候 可能要修改 因为 udp tcp 有区别
		if (sendto(poSock->GetSock(), (char*)(&oUDPPacketHeader),
			sizeof(oUDPPacketHeader), 0, (sockaddr*)(&pstPerIoData->stRemoteAddr),
			sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "sendto errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				closesocket(hSock);
				PostAccept(*pstPerIoData);
				FxMySockMgr::Instance()->ReleaseUdpSock(poSock);
				FxConnectionMgr::Instance()->Release(poConnection);
				return;
			}
		}
		//poSock->Send((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));

		if (false == poSock->AddEvent())
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->AddEvent failed");

			poSock->Close();
		}

		PostAccept(*pstPerIoData);
	}
}

#else
void FxUDPListenSock::OnAccept()
{

}

#endif // WIN32

FxUDPConnectSock::FxUDPConnectSock()
{
	m_poSendBuf = NULL;
	m_poRecvBuf = NULL;
	m_poConnection = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_nNeedData = 0;
	m_nPacketLen = 0;
#ifdef WIN32
	m_dwLastError = 0;
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

FxUDPConnectSock::~FxUDPConnectSock()
{
	if (m_poConnection)
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		m_poConnection->OnSocketDestroy();
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

bool FxUDPConnectSock::Init()
{
	m_cSyn = 1;
	m_cAck = 0;
	if (!m_oEvtQueue.Init(MAX_NETEVENT_PERSOCK))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_oEvtQueue.Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (NULL == m_poSendBuf)
	{
		m_poSendBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poSendBuf)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "NULL == m_poSendBuf, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
	}

	if (NULL == m_poRecvBuf)
	{
		m_poRecvBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poRecvBuf)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "NULL == m_poRecvBuf, socket : %d, socket id : %d", GetSock(), GetSockId());

			return false;
		}
	}

	if (!m_poRecvBuf->Init(RECV_BUFF_SIZE))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_poRecvBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (!m_poSendBuf->Init(SEND_BUFF_SIZE))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_poSendBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}
	return true;
}

void FxUDPConnectSock::OnRead()
{

}

void FxUDPConnectSock::OnWrite()
{

}

void FxUDPConnectSock::Reset()
{
	if (NULL != m_poConnection)
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		m_poConnection->OnSocketDestroy();
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
	m_dwLastError = 0;
	m_nPostRecv = 0;
	m_nPostSend = 0;
	m_dwLastError = 0;
#else
	m_bSending = false;
#endif // WIN32
	m_bSendLinger = false;     //发送延迟，直到成功，或者30次后，这时默认设置//
}

bool FxUDPConnectSock::Close()
{
	// 首先 把数据先发过去//
	m_oLock.Lock();

	if (GetState() == SSTATE_RELEASE || GetState() == SSTATE_CLOSE)
	{
		m_oLock.UnLock();
		return true;
	}

	if (IsConnected())
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
	// 有bug 就不发了 修改后再说//
	//SendImmediately();
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

bool FxUDPConnectSock::Send(const char* pData, int dwLen)
{
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "socket not connected, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (m_poSendBuf->IsEmpty())
	{
		m_poSendBuf->Clear();
	}

	IFxDataHeader* pDataHeader = GetDataHeader();
	if (pDataHeader == NULL)
	{
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogFun(LT_Screen | LT_File, LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogFun(LT_Screen | LT_File, LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		Close();
#endif // WIN32
		return false;
	}

	if (dwLen + pDataHeader->GetHeaderLength() + sizeof(UDPPacketHeader) > m_poSendBuf->GetTotalLen())
	{
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogFun(LT_Screen | LT_File, LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogFun(LT_Screen | LT_File, LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		Close();
#endif // WIN32
		return false;
	}

	// 这个是在主线程调用 所以 声明为静态就可以了 防止重复生成 占用空间
	static char pTemData[RECV_BUFF_SIZE] = { 0 };
	UDPPacketHeader oUDPPacketHeader = {0};
	// todo syn ack 数值到底如何定义
	CNetStream oNetStream(ENetStreamType_Write, pTemData, dwLen + pDataHeader->GetHeaderLength());
	oNetStream.WriteData((char*)(pDataHeader->BuildSendPkgHeader(dwLen + sizeof(oUDPPacketHeader))), pDataHeader->GetHeaderLength());
	oNetStream.WriteData((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));
	oNetStream.WriteData(pData, dwLen);

	int nSendCount = 0;
	while (!m_poSendBuf->PushBuff(pTemData, dwLen + pDataHeader->GetHeaderLength()))
	{
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			LogFun(LT_Screen | LT_File, LogLv_Critical, "send buffer overflow!!!!!!!!, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
		FxSleep(10);
	}

	if (false == PostSendFree())
	{
#ifdef WIN32
		m_dwLastError = WSAGetLastError();
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostSendFree(), socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostSendFree(), socket : %d, socket id : %d", GetSock(), GetSockId());

		Close();
#endif // WIN32
		return false;
	}

	return true;
}

bool FxUDPConnectSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{

	return false;
}

IFxDataHeader* FxUDPConnectSock::GetDataHeader()
{
	return NULL;
}

bool FxUDPConnectSock::AddEvent()
{

	return false;
}

void FxUDPConnectSock::ProcEvent()
{

}

SOCKET FxUDPConnectSock::Connect()
{
	return INVALID_SOCKET;
}

#ifdef WIN32
bool FxUDPConnectSock::PostRecv()
{
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//LogFun(LT_Screen | LT_File, LogLv_Error, "m_poRecvBuf->GetInCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());

		// 接受缓存不够 等会继续接收 //
		InterlockedCompareExchange(&m_nPostRecv, 0, 1);
		PushNetEvent(NETEVT_RECV, -1);
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;
	m_stRecvIoData.stWsaBuf.len = nLen;

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	int nSockAddr = sizeof(m_stRecvIoData.stRemoteAddr);
	if (SOCKET_ERROR == WSARecvFrom(GetSock(), &m_stRecvIoData.stWsaBuf, 1, &dwReadLen, &dwFlags,
		(sockaddr*)(&m_stRecvIoData.stWsaBuf), &nSockAddr, &m_stRecvIoData.stOverlapped, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "WSARecvFrom errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			InterlockedCompareExchange(&m_nPostRecv, 0, 1);
			return false;
		}
	}

	return true;
}

bool FxUDPConnectSock::PostClose()
{

	return false;
}

bool FxUDPConnectSock::PostRecvFree()
{

	return false;
}

void FxUDPConnectSock::OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred)
{
	SPerUDPIoData* pSPerUDPIoData = (SPerUDPIoData*)pIoData;

	switch (GetState())
	{
	case SSTATE_ESTABLISH:
	{
		// todo
	}
	break;
	case SSTATE_CONNECT:
	{
		// connect后 收到的第一条消息肯定是服务器发过来的确认
		if (dwByteTransferred < sizeof(UDPPacketHeader))
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "recv size error");
			Close();
			return;
		}
		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)pSPerUDPIoData->stWsaBuf.buf;
		if (pUDPPacketHeader->m_cAck != 1)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "ack error want : 1, recv : %d", pUDPPacketHeader->m_cAck);
			Close();
			return;
		}
		if (pUDPPacketHeader->m_cSyn!= 1)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "syn error want : 1, recv : %d", pUDPPacketHeader->m_cSyn);
			Close();
			return;
		}
		if (pUDPPacketHeader->m_cStatus != SSTATE_ESTABLISH)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "statu error want : SSTATE_ESTABLISH, recv : %d", pUDPPacketHeader->m_cStatus);
			Close();
			return;
		}

		sockaddr_in stLocalAddr;
		INT32 nLocalAddrLen = sizeof(sockaddr_in);

		if (getsockname(GetSock(), (sockaddr*)(&stLocalAddr), &nLocalAddrLen) == SOCKET_ERROR)
		{
			int dwErr = WSAGetLastError();
			LogFun(LT_Screen | LT_File, LogLv_Error, "getsockname error: %d", dwErr);

			Close();
			return;
		}

		GetConnection()->SetLocalIP(stLocalAddr.sin_addr.s_addr);
		GetConnection()->SetLocalPort(ntohs(stLocalAddr.sin_port));

		GetConnection()->SetRemoteIP(pSPerUDPIoData->stRemoteAddr.sin_addr.s_addr);
		GetConnection()->SetRemotePort(ntohs(pSPerUDPIoData->stRemoteAddr.sin_port));

		//
		// 应该先投递连接事件再关联套接口，否则可能出现第一个Recv事件先于连接事件入队列
		//
		if (connect(GetSock(), (sockaddr*)(&pSPerUDPIoData->stRemoteAddr), sizeof(pSPerUDPIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				Close();
				return;
			}
		}
		{
			SetState(SSTATE_ESTABLISH);
			PushNetEvent(NETEVT_ESTABLISH, 0);			// 到了这个时候才算是连接成功

			if (false == PostRecv())
			{
				int dwErr = WSAGetLastError();
				PushNetEvent(NETEVT_ERROR, dwErr);
				LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->PostRecv failed, errno : %d", dwErr);

				Close();
				return;
			}
		}
	}
	break;
	default:
	{
		// 如果其他状态收到消息 肯定不对
		LogFun(LT_Screen | LT_File, LogLv_Error, "state : %d, error", (UINT32)GetState());
		Close();        // 未知错误，不应该发生//
	}
		break;
	}
}
#else
void FxUDPConnectSock::OnParserIoEvent(int dwEvents)
{

}

#endif // WIN32

bool FxUDPConnectSock::PostSend()
{

	return false;
}

bool FxUDPConnectSock::PostSendFree()
{

	return false;
}

bool FxUDPConnectSock::SendImmediately()
{

	return false;
}

void FxUDPConnectSock::__ProcEstablish()
{

}

void FxUDPConnectSock::__ProcAssociate()
{

}

void FxUDPConnectSock::__ProcConnectError(UINT32 dwErrorNo)
{

}

void FxUDPConnectSock::__ProcError(UINT32 dwErrorNo)
{

}

void FxUDPConnectSock::__ProcTerminate()
{

}

void FxUDPConnectSock::__ProcRecv(UINT32 dwLen)
{

}

void FxUDPConnectSock::__ProcRelease()
{

}

void FxUDPConnectSock::OnConnect()
{

}

#ifdef WIN32
void FxUDPConnectSock::OnRecv(bool bRet, int dwBytes)
{

}

void FxUDPConnectSock::OnSend(bool bRet, int dwBytes)
{

}

#else
void FxUDPConnectSock::OnRecv()
{

}
void FxUDPConnectSock::OnSend()
{

}


#endif // WIN32

