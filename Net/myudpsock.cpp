#include "myudpsock.h"
#include "sockmgr.h"
#include <stdio.h>
#include "connectionmgr.h"
#include "connection.h"
#include "iothread.h"
#include "net.h"
#include "netstream.h"

FxUDPListenSock::FxUDPListenSock()
{
	Reset();

	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_poSessionFactory = NULL;
	m_poRecvBuf = NULL;
}

FxUDPListenSock::~FxUDPListenSock()
{

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

	if (m_poRecvBuf->IsEmpty())
	{
		m_poRecvBuf->Clear();
	}

	memset(&oSPerIoData.stOverlapped, 0, sizeof(oSPerIoData.stOverlapped));
	int nLen = m_poRecvBuf->GetInCursorPtr(oSPerIoData.stWsaBuf.buf);
	if (0 >= nLen)
	{
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;
	oSPerIoData.stWsaBuf.len = nLen;

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

bool FxUDPListenSock::InitAcceptEx()
{
	// todo
	return false;
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
			LogFun(LT_Screen | LT_File, LogLv_Error, "Set keep alive error: %d", dwErr);

			PostAccept(*pstPerIoData);
			poSock->PushNetEvent(NETEVT_ERROR, dwErr);
			poSock->Close();
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

		// todo  这个时候不能说是已经establish 了 要发个消息确认下
		UDPPacketHeader oUDPPacketHeader;
		oUDPPacketHeader.m_cAck = 1;
		oUDPPacketHeader.m_cSyn = ((UDPPacketHeader*)(pstPerIoData->stWsaBuf.buf))->m_cSyn;
		oUDPPacketHeader.m_cStatus = poSock->GetState();

		// todo send的时候 可能要修改 因为 udp tcp 有区别
		poSock->Send((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));

		////
		//// 应该先投递连接事件再关联套接口，否则可能出现第一个Recv事件先于连接事件入队列
		////

		//if (false == poSock->AddEvent())
		//{
		//	LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->AddEvent failed");

		//	poSock->Close();
		//}
		//else
		//{
		//	poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

		//	if (false == poSock->PostRecv())
		//	{
		//		int dwErr = WSAGetLastError();
		//		poSock->PushNetEvent(NETEVT_ERROR, dwErr);
		//		LogFun(LT_Screen | LT_File, LogLv_Error, "poSock->PostRecv failed, errno : %d", dwErr);

		//		poSock->Close();
		//	}
		//}

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

}

FxUDPConnectSock::~FxUDPConnectSock()
{

}

bool FxUDPConnectSock::Init()
{

	return false;
}

void FxUDPConnectSock::OnRead()
{

}

void FxUDPConnectSock::OnWrite()
{

}

void FxUDPConnectSock::Reset()
{

}

bool FxUDPConnectSock::Close()
{

	return false;
}

bool FxUDPConnectSock::Send(const char* pData, int dwLen)
{

	return false;
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

	return false;
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

