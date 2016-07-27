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

#define MAX_LOST_PACKET 10

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
	SOCKET hNewSock = WSASocket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP,
		NULL,
		0,
		WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == hNewSock)
	{
		int dwErr = WSAGetLastError();
		LogFun(LT_Screen | LT_File, LogLv_Error, "WSASocket failed, errno %d", dwErr);
		return false;
	}
	oSPerIoData.hSock = hNewSock;
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

		poConnection->SetSockType(SOCKTYPE_UDP);
		poConnection->SetSock(poSock);
		poConnection->SetID(poSock->GetSockId());

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

		// 这个时候不能说是已经establish 了 要发个消息确认下
		UDPPacketHeader oUDPPacketHeader = {0};
		oUDPPacketHeader.m_cAck = ((UDPPacketHeader*)(pstPerIoData->stWsaBuf.buf))->m_cSyn;
		oUDPPacketHeader.m_cSyn = 1;
		oUDPPacketHeader.m_cStatus = poSock->GetState();

		// send的时候 可能要修改 因为 udp tcp 有区别
		if (sendto(poSock->GetSock(), (char*)(&oUDPPacketHeader),
			sizeof(oUDPPacketHeader), 0, (sockaddr*)(&pstPerIoData->stRemoteAddr),
			sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "sendto errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				PostAccept(*pstPerIoData);
				poSock->Close();
				return;
			}
		}
		//poSock->Send((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));

		sockaddr_in stLocalAddr;
		INT32 nLocalAddrLen = sizeof(sockaddr_in);
		
		if (getsockname(poSock->GetSock(), (sockaddr*)(&stLocalAddr), &nLocalAddrLen) == SOCKET_ERROR)
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

		poSock->SetRemoteAddr(pstPerIoData->stRemoteAddr);

		if (false == poSock->AddEvent())
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->AddEvent failed");

			poSock->Close();
		}
		poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

		if (false == poSock->PostRecv())
		{
			int dwErr = WSAGetLastError();
			poSock->PushNetEvent(NETEVT_ERROR, dwErr);
			LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->PostRecv failed, errno : %d", dwErr);

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
	Reset();
	m_cSyn = 1;
	m_cAck = 0;

	memset(&m_stRecvIoData.stRemoteAddr, 0, sizeof(m_stRecvIoData.stRemoteAddr));
	memset(&m_stSendIoData.stRemoteAddr, 0, sizeof(m_stSendIoData.stRemoteAddr));
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
	if (NULL != GetConnection())
	{
		// 既然是要销毁 那么应该通知 将相应指针置零//
		GetConnection()->OnSocketDestroy();
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
	CancelIo((HANDLE)GetSock());
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

	if ((unsigned int)dwLen + pDataHeader->GetHeaderLength() + sizeof(UDPPacketHeader) > (unsigned int)m_poSendBuf->GetTotalLen())
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
	oUDPPacketHeader.m_cSyn = ++m_cSyn;
	oUDPPacketHeader.m_cAck = m_cAck;
	oUDPPacketHeader.m_cStatus = (char)GetState();

	CNetStream oNetStream(ENetStreamType_Write, pTemData, dwLen + pDataHeader->GetHeaderLength() + sizeof(oUDPPacketHeader));
	oNetStream.WriteData((char*)(pDataHeader->BuildSendPkgHeader(dwLen + sizeof(oUDPPacketHeader))), pDataHeader->GetHeaderLength());
	oNetStream.WriteData((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));
	oNetStream.WriteData(pData, dwLen);

	int nSendCount = 0;
	while (!m_poSendBuf->PushBuff(pTemData, dwLen + pDataHeader->GetHeaderLength() + sizeof(oUDPPacketHeader)))
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

IFxDataHeader* FxUDPConnectSock::GetDataHeader()
{
	if (GetConnection())
	{
		return GetConnection()->GetDataHeader();
	}
	return NULL;
}

bool FxUDPConnectSock::AddEvent()
{
#ifdef WIN32
	if (!m_poIoThreadHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		LogFun(LT_Screen | LT_File, LogLv_Error, "error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		Close();
		return false;
	}
#else
	if (!m_poIoThreadHandler->AddEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		LogFun(LT_Screen | LT_File, LogLv_Error, "error : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		Close();
		return false;
	}
#endif // WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;
}

void FxUDPConnectSock::ProcEvent()
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
				Assert(0);
			}
			break;
		}
	}
}

SOCKET FxUDPConnectSock::Connect()
{
	if (IsConnected())
	{
		return INVALID_SOCKET;
	}

	if (NULL == GetConnection())
	{
		return INVALID_SOCKET;
	}

	if (GetConnection()->IsConnected())
	{
		return INVALID_SOCKET;
	}

	if (INVALID_SOCKET != GetSock())
	{
		return INVALID_SOCKET;
	}

	SetSock(socket(AF_INET, SOCK_DGRAM, 0));
	if (INVALID_SOCKET == GetSock())
	{
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return INVALID_SOCKET;
	}

#ifdef WIN32
	int nSendBuffSize = 256 * 1024;
	int nRecvBuffSize = 8 * nSendBuffSize;
	if ((0 != setsockopt(GetSock(), SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuffSize, sizeof(int))) &&
		(0 != setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuffSize, sizeof(int))))
	{
		int nError = WSAGetLastError();
		PushNetEvent(NETEVT_CONN_ERR, nError);
		closesocket(GetSock());
		return INVALID_SOCKET;
	}

#else
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDLOWAT, &VAL_SO_SNDLOWAT, sizeof(VAL_SO_SNDLOWAT));
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, &MAX_SYS_SEND_BUF, sizeof(MAX_SYS_SEND_BUF));
#endif // WIN32

	SetIoThread(FxNetModule::Instance()->FetchIoThread(GetSockId()));
	if (NULL == m_poIoThreadHandler)
	{
		PushNetEvent(NETEVT_ERROR, 0);
		Close();
		return INVALID_SOCKET;
	}

	sockaddr_in stLocalAddr = { 0 };
	stLocalAddr.sin_family = AF_INET;
	stLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stLocalAddr.sin_port = 0;
	if (bind(GetSock(), (sockaddr*)&stLocalAddr, sizeof(stLocalAddr)) == SOCKET_ERROR)
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // WIN32
		LogFun(LT_Screen | LT_File, LogLv_Error, "connect bind failed, errno %d", dwErr);
		return INVALID_SOCKET;
	}

	INT32 nLocalAddrLen = sizeof(stLocalAddr);
	if (getsockname(GetSock(), (sockaddr*)&stLocalAddr, &nLocalAddrLen) == SOCKET_ERROR)
	{
		PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
		closesocket(GetSock());
		LogFun(LT_Screen | LT_File, LogLv_Error, "socket getsockname error : %d, socket : %d, socket id %d", WSAGetLastError(), GetSock(), GetSockId());
		return INVALID_SOCKET;
	}

	GetConnection()->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	GetConnection()->SetLocalPort(ntohs(stLocalAddr.sin_port));

	sockaddr_in stAddr = { 0 };
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = GetConnection()->GetRemoteIP();
	stAddr.sin_port = htons(GetConnection()->GetRemotePort());

	SetState(SSTATE_CONNECT);

	UDPPacketHeader oUDPPacketHeader = { 0 };
	oUDPPacketHeader.m_cAck = 1;
	oUDPPacketHeader.m_cSyn = 1;
	oUDPPacketHeader.m_cStatus = GetState();

	if (sendto(GetSock(), (char*)(&oUDPPacketHeader),
		sizeof(oUDPPacketHeader), 0, (sockaddr*)(&stAddr),
		sizeof(stAddr)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "sendto errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}

	sockaddr_in stRemoteAddr = { 0 };
	int nRemoteAddrLen = sizeof(stRemoteAddr);

	oUDPPacketHeader = {0};
	if (recvfrom(GetSock(), (char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader), 0, (sockaddr*)&stRemoteAddr, &nRemoteAddrLen))
	{
		if (oUDPPacketHeader.m_cAck != 1)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "ack error want : 1, recv : %d", oUDPPacketHeader.m_cAck);
			Close();
			return INVALID_SOCKET;
			}
		if (oUDPPacketHeader.m_cSyn != 1)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "syn error want : 1, recv : %d", oUDPPacketHeader.m_cSyn);
			Close();
			return INVALID_SOCKET;
		}
		if (oUDPPacketHeader.m_cStatus != SSTATE_ESTABLISH)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "statu error want : SSTATE_ESTABLISH, recv : %d", oUDPPacketHeader.m_cStatus);
			Close();
			return INVALID_SOCKET;
		}

		SetRemoteAddr(stRemoteAddr);
	}

#ifdef WIN32
	{
		m_stRecvIoData.nOp = IOCP_RECV;
		unsigned long ul = 1;
		if (SOCKET_ERROR == ioctlsocket(GetSock(), FIONBIO, (unsigned long*)&ul))
		{
			PushNetEvent(NETEVT_CONN_ERR, (UINT32)WSAGetLastError());
			closesocket(GetSock());
			LogFun(LT_Screen | LT_File, LogLv_Error, "Set socket FIONBIO error : %d, socket : %d, socket id %d", WSAGetLastError(), GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}
#endif // WIN32

	if (!AddEvent())
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // WIN32
		PushNetEvent(NETEVT_CONN_ERR, dwErr);
		closesocket(GetSock());
		return INVALID_SOCKET;
	}
	OnConnect();
	return GetSock();
}

void FxUDPConnectSock::SetRemoteAddr(sockaddr_in& refstRemoteAddr)
{
	memcpy(&m_stRecvIoData.stRemoteAddr, &refstRemoteAddr, sizeof(refstRemoteAddr));
	memcpy(&m_stSendIoData.stRemoteAddr, &refstRemoteAddr, sizeof(refstRemoteAddr));
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
	if (2048 >= nLen)
	{
		//LogFun(LT_Screen | LT_File, LogLv_Error, "m_poRecvBuf->GetInCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());

		// 接受缓存不够 等会继续接收 udp与tcp不一样 一个包必须一次接完 不然会出问题 就是错误码为234的错误 接收长度后期调整//
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
		(sockaddr*)(&m_stRecvIoData.stRemoteAddr), &nSockAddr, &m_stRecvIoData.stOverlapped, NULL))
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
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	// Post失败的时候再进入这个函数时可能会丢失一次//

	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), UINT32(0), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "PostQueuedCompletionStatus errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		return false;
	}

	return true;
}

bool FxUDPConnectSock::PostRecvFree()
{
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());
		return false;
	}
	LONG nPostRecv = InterlockedCompareExchange(&m_nPostRecv, 1, 0);
	if (0 != nPostRecv)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_nPostRecv == 1, socket : %d, socket id : %d", GetSock(), GetSockId());
		return false;
	}
	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	//post 失败的时候 再进入这个函数 可能会丢失一次
	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), UINT32(-1), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "PostQueuedCompletionStatus error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());
		InterlockedCompareExchange(&m_nPostRecv, 0, 1);
		return false;
	}

	return true;
}

void FxUDPConnectSock::OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred)
{
	SPerUDPIoData* pSPerUDPIoData = (SPerUDPIoData*)pIoData;
	if (NULL == pSPerUDPIoData)
	{
		Close();
		return;
	}

	switch (GetState())
	{
		case SSTATE_ESTABLISH:
		{
			switch (pSPerUDPIoData->nOp)
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
						Assert(0);
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
			if (pUDPPacketHeader->m_cSyn != 1)
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

			SetRemoteAddr(pSPerUDPIoData->stRemoteAddr);
			OnConnect();
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
#ifdef WIN32
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnect(), socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//LogFun(LT_Screen | LT_File, LogLv_Critical, "m_poSendBuf->GetOutCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());
		// 不算失败，只是没有投递而已，下次可以继续//
		InterlockedCompareExchange(&m_nPostSend, 0, 1);
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;     // ???64K

	m_stSendIoData.stWsaBuf.len = nLen;
	DWORD dwNumberOfBytesSent = 0;

	int nRet = WSASendTo(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
		(sockaddr*)(&(m_stSendIoData.stRemoteAddr)), sizeof((m_stSendIoData.stRemoteAddr)), &m_stSendIoData.stOverlapped, NULL);
	if (0 != nRet)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			InterlockedCompareExchange(&m_nPostSend, 0, 1);

			UINT32 dwErr = WSAGetLastError();
			LogFun(LT_Screen | LT_File, LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			return false;
		}
	}

	return true;
#else
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "NULL == m_poIoThreadHandler, socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//		LogFun(LT_Screen | LT_File, LogLv_Error, "m_poSendBuf->GetOutCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());
		// 不算失败，只是没有投递而已，下次可以继续//
		// modify: 在这里需要设置事件为EPOLLIN，否则OUT事件一直被设置，导致CPU很高//
		if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

			PushNetEvent(NETEVT_ERROR, errno);
			Close();
			return false;
		}
		return true;
	}

	m_bSending = true;
	nLen = VAL_SO_SNDLOWAT < nLen ? VAL_SO_SNDLOWAT : nLen;     // ���64K

	int nRet = send(GetSock(), pSendBuf, nLen, 0);
	if (0 > nRet)
	{
		if (EAGAIN == errno)
		{
			if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

				m_bSending = false;
				return false;
			}
			return true;
		}

		LogFun(LT_Screen | LT_File, LogLv_Error, "send nRet < 0, socket : %d, socket id : %d", GetSock(), GetSockId());

		m_bSending = false;
		return false;
	}
	else if (0 == nRet)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "send 0 == nRet, socket : %d, socket id : %d", GetSock(), GetSockId());

		m_bSending = false;
		return false;
	}

	// 把成功发送了的从发送缓冲区丢弃//
	m_poSendBuf->DiscardBuff(nRet);

	if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

		m_bSending = false;
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return false;
	}

	return true;
#endif // WIN32
}

bool FxUDPConnectSock::PostSendFree()
{
#ifdef WIN32
	return PostSend();
#else
	if (false == IsConnected())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "NULL == m_poIoThreadHandler, socket : %d, socket id : %d", GetSock(), GetSockId());

		Close();
		return false;
	}

	return m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this);

	return true;
#endif // WIN32
}

bool FxUDPConnectSock::SendImmediately()
{
	// todo
	return false;
}

void FxUDPConnectSock::__ProcEstablish()
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		GetConnection()->OnConnect();
	}
}

void FxUDPConnectSock::__ProcAssociate()
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		GetConnection()->OnAssociate();
	}
}

void FxUDPConnectSock::__ProcConnectError(UINT32 dwErrorNo)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		GetConnection()->OnConnError(dwErrorNo);
	}
}

void FxUDPConnectSock::__ProcError(UINT32 dwErrorNo)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		GetConnection()->OnError(dwErrorNo);
	}
}

void FxUDPConnectSock::__ProcTerminate()
{
	PushNetEvent(NETEVT_RELEASE, 0);
}

void FxUDPConnectSock::__ProcRecv(UINT32 dwLen)
{
	if (GetConnection())
	{
		if (UINT32(-1) == dwLen)
		{
#ifdef WIN32
			PostRecvFree();
#else
			PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
			Close();
#endif
			return;
		}

		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		if (GetConnection()->GetRecvSize() < dwLen)
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "session recv_size error need : %d, but : %d", dwLen, GetConnection()->GetRecvSize());
			PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
#ifdef WIN32
			PostClose();
#else
			Close();
#endif // WIN32
			return;
		}

		if (!m_poRecvBuf->PopBuff(GetConnection()->GetRecvBuf(), dwLen))
		{
			return;
		}

		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)(GetConnection()->GetRecvBuf() + GetDataHeader()->GetHeaderLength());
		// 参数一定要是syn
		if (!IsValidAck(pUDPPacketHeader->m_cSyn))
		{
#ifdef WIN32
			PostClose();
#else
			Close();
#endif // WIN32
			return;
		}

		GetConnection()->OnRecv(dwLen);
	}
}

void FxUDPConnectSock::__ProcRelease()
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnSocketDestroy();
		GetConnection()->OnClose();

		SetConnection(NULL);
	}
	FxMySockMgr::Instance()->ReleaseUdpSock(this);
}

void FxUDPConnectSock::OnConnect()
{
	GetConnection()->SetID(GetSockId());

	SetState(SSTATE_ESTABLISH);
	PushNetEvent(NETEVT_ESTABLISH, 0);

#ifdef WIN32
	if (false == PostRecv())
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostRecv(), socket : %d, socket id : %d", GetSock(), GetSockId());

		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		Close();
	}
#else
	INT32 nError = 0;
	socklen_t nLen = sizeof(nError);
	if (getsockopt(GetSock(), SOL_SOCKET, SO_ERROR, &nError, &nLen) < 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "getsockopt errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		PushNetEvent(NETEVT_CONN_ERR, errno);
		Close();
		return;
	}

	if (nError != 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "getsockopt errno : %d, socket : %d, socket id : %d", nError, GetSock(), GetSockId());

		PushNetEvent(NETEVT_CONN_ERR, nError);
		Close();
		return;
	}

	if (!m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

		PushNetEvent(NETEVT_CONN_ERR, errno);
		Close();
	}
#endif // WIN32
}

bool FxUDPConnectSock::IsValidAck(char cAck)
{
	if (cAck <= m_cAck)
	{
		if (cAck >= (m_cAck + MAX_LOST_PACKET))
		{
			return false;
		}
	}
	if (cAck >= (m_cAck + MAX_LOST_PACKET))
	{
		if (cAck <= m_cAck)
		{
			return false;
		}
	}
	
	m_cAck = cAck;
	return true;
}

#ifdef WIN32
void FxUDPConnectSock::OnRecv(bool bRet, int dwBytes)
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
		int nErr = WSAGetLastError();
		LogFun(LT_Screen | LT_File, LogLv_Error, "false == bRet errno : %d, socket : %d, socket id : %d", nErr, GetSock(), GetSockId());

		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		m_dwLastError = nErr;
		PostClose();
		return;
	}

	if (!IsConnected())
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
	}

	char *pUseBuf = NULL;
	nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
	if (nLen <= 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "m_poRecvBuf->GetUsedCursorPtr <= 0, socket : %d, socket id : %d", GetSock(), GetSockId());

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
			GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen > nLen ? nLen : dwHeaderLen, 0);
			m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
			if (-1 == m_nPacketLen)
			{
				LogFun(LT_Screen | LT_File, LogLv_Error, "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

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
							LogFun(LT_Screen | LT_File, LogLv_Error, "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

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
							LogFun(LT_Screen | LT_File, LogLv_Error, "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

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
			LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostRecv, socket : %d, socket id : %d", GetSock(), GetSockId());

			m_dwLastError = WSAGetLastError();
			PostClose();
		}
	}
	m_oLock.UnLock();
}

void FxUDPConnectSock::OnSend(bool bRet, int dwBytes)
{
	if (false == bRet)
	{
		InterlockedCompareExchange(&m_nPostSend, 0, m_nPostSend);
		m_dwLastError = WSAGetLastError();
		PostClose();
		return;
	}

	if (false == IsConnected())
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
				LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostSend, socket : %d, socket id : %d", GetSock(), GetSockId());

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

	// 把成功发送了的从发送缓冲区丢弃//
	m_poSendBuf->DiscardBuff(dwBytes);

	InterlockedCompareExchange(&m_nPostSend, m_nPostSend - 1, m_nPostSend);
	if (0 == m_nPostSend)
	{
		if (false == PostSend())
		{
			LogFun(LT_Screen | LT_File, LogLv_Error, "false == PostSend, socket : %d, socket id : %d", GetSock(), GetSockId());

			m_dwLastError = WSAGetLastError();
			PostClose();
		}
	}
}

#else
void FxUDPConnectSock::OnRecv()
{

}
void FxUDPConnectSock::OnSend()
{

}


#endif // WIN32

