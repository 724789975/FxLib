#include "myudpsock.h"
#include "sockmgr.h"
#include <stdio.h>
#include "connectionmgr.h"
#include "connection.h"
#include "iothread.h"
#include "net.h"
#include "netstream.h"
#include <math.h>

#define RECV_BUFF_SIZE 32*64*1024
#define SEND_BUFF_SIZE 32*64*1024

#ifdef _WIN32
#else
#include <unistd.h>
#include <fcntl.h>
int UDP_MAX_SYS_SEND_BUF = (128 * 1024);
int UDP_VAL_SO_SNDLOWAT = (64 * 1024);
#endif // _WIN32

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
	return true;
}

void FxUDPListenSock::OnRead()
{

}

void FxUDPListenSock::OnWrite()
{

}

SOCKET FxUDPListenSock::Listen(unsigned int dwIP, unsigned short& wPort)
{
	memset(&m_stAddr, 0, sizeof(m_stAddr));
	SetSock(socket(AF_INET, SOCK_DGRAM, 0));
	if (INVALID_SOCKET == GetSock())
	{
#ifdef _WIN32
		int dwErr = WSAGetLastError();
		LogExe(LogLv_Error, "create socket error, %u:%u, errno %d", dwIP, wPort, dwErr);
#else
		LogExe(LogLv_Error, "create socket error, %u:%u, errno %d", dwIP, wPort, errno);
#endif // _WIN32

		return false;
	}

	int nReuse = 1;
	setsockopt(GetSock(), SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof(nReuse));

	m_stAddr.sin_family = AF_INET;
	if (0 == dwIP)
	{
		m_stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		m_stAddr.sin_addr.s_addr = dwIP;
	}
	m_stAddr.sin_port = htons(wPort);

#ifdef _WIN32
	unsigned long ul = 1;
	if (SOCKET_ERROR == ioctlsocket(GetSock(), FIONBIO, (unsigned long*)&ul))
	{
		closesocket(GetSock());
		LogExe(LogLv_Error, "Set socket FIONBIO error : %d, socket : %d, socket id %d",
			WSAGetLastError(), GetSock(), GetSockId());
		return false;
	}
#endif // _WIN32

	if (bind(GetSock(), (sockaddr*)&m_stAddr, sizeof(m_stAddr)) < 0)
	{
#ifdef _WIN32
		int dwErr = WSAGetLastError();
		LogExe(LogLv_Error, "bind at %u:%d failed, errno %d", dwIP, wPort, dwErr);
#else
		LogExe(LogLv_Error, "bind at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // _WIN32
		return false;
	}

	m_poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(GetSock());
	if (NULL == m_poIoThreadHandler)
	{
		return false;
	}

	SetState(SSTATE_LISTEN);

	// 添加到事件 //
	if (false == AddEvent())
	{
		return false;
	}
	LogExe(LogLv_Info, "listen at %u:%d success", dwIP, wPort);

#ifdef _WIN32
	for (int i = 0; i < sizeof(m_oSPerIoDatas) / sizeof(SPerUDPIoData); ++i)
	{
		m_oSPerIoDatas[i].stWsaBuf.buf = (char*)(&m_oPacketHeaders[i]);
		m_oSPerIoDatas[i].stWsaBuf.len = sizeof(m_oPacketHeaders[i]);
		PostAccept(m_oSPerIoDatas[i]);
	}
#endif // _WIN32
	return true;
}

bool FxUDPListenSock::StopListen()
{
	if (SSTATE_LISTEN != GetState())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "state : %d != SSTATE_LISTEN", (unsigned int)GetState());
		return false;
	}

	if (INVALID_SOCKET == GetSock())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "socket : %d == INVALID_SOCKET", GetSock());
		return false;
	}

#ifdef _WIN32
	shutdown(GetSock(), SD_RECEIVE);
#else
	shutdown(GetSock(), SHUT_RD);
#endif // _WIN32

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
#ifdef _WIN32
	closesocket(GetSock());
#else
	m_poIoThreadHandler->DelEvent(GetSock());
	close(GetSock());
#endif // _WIN32

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

bool FxUDPListenSock::PushNetEvent(ENetEvtType eType, unsigned int dwValue)
{
	if (SSTATE_INVALID == GetState())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "state : %d == SSTATE_INVALID", (unsigned int)GetState());

		return false;
	}
	SNetEvent oEvent;
	// 先扔网络事件进去，然后在报告上层有事件，先后顺序不能错，这样上层就不会错取事件//
	oEvent.eType = eType;
	oEvent.dwValue = dwValue;

	while (!FxNetModule::Instance()->PushNetEvent(this, oEvent))
	{
		FxSleep(1);
	}

	return true;
}

void FxUDPListenSock::ProcEvent(SNetEvent oEvent)
{
	{
		switch (oEvent.eType)
		{
		case NETEVT_ASSOCIATE:
		{
			__ProcAssociate();
		}
		break;

		case NETEVT_ERROR:
		{
			__ProcError(oEvent.dwValue);
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

#ifdef _WIN32
void FxUDPListenSock::OnParserIoEvent(bool bRet, void* pIoData, unsigned int dwByteTransferred)
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "OnParserIoEvent failed, errno %d", dwErr);

			closesocket(pSPerUDPIoData->hSock);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}
		if (dwByteTransferred != sizeof(UDPPacketHeader))
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "%s", "recv length error !!!");
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)pSPerUDPIoData->stWsaBuf.buf;

		if ((ESocketState)pUDPPacketHeader->m_cStatus != SSTATE_CONNECT)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv socket state error want : %d, recv : %d !!!",
				SSTATE_CONNECT, pUDPPacketHeader->m_cStatus);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		if (pUDPPacketHeader->m_cSyn != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv socket syn error want : %d, recv : %d !!!",
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "%s", "listen socket has stoped but ret is true");
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "state : %d != SSTATE_LISTEN", (unsigned int)GetState());

		PushNetEvent(NETEVT_ERROR, NET_UNKNOWN_ERROR);
		Close();        // 未知错误，不应该发生//
	}
	break;
	}
}
#else
void FxUDPListenSock::OnParserIoEvent(int dwEvents)
{
	if (dwEvents & EPOLLERR)
	{
		PushNetEvent(NETEVT_ERROR, errno);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "get error event errno : %d", errno);

		Close();
		return;
	}

	if (dwEvents & EPOLLIN)
	{
		OnAccept();
	}
}
#endif // _WIN32

bool FxUDPListenSock::AddEvent()
{
#ifdef _WIN32
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
#endif // _WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;
}

void FxUDPListenSock::__ProcAssociate()
{

}

void FxUDPListenSock::__ProcError(unsigned int dwErrorNo)
{

}

void FxUDPListenSock::__ProcTerminate()
{

}

#ifdef _WIN32
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "WSASocket failed, errno %d", dwErr);
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "WSARecvFrom errno : %d, socket : %d, socket id : %d",
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "state : %d != SSTATE_LISTEN", GetState());

		closesocket(hSock);
		return;
	}

	{
		FxUDPConnectSock* poSock = FxMySockMgr::Instance()->CreateUdpSock();
		if (NULL == poSock)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, create CCPSock failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
		if (NULL == poConnection)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, create Connection failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			return;
		}

		poSock->SetSock(hSock);
		poSock->SetConnection(poConnection);

		poConnection->SetSockType(SLT_Udp);
		poConnection->SetSock(poSock);
		poConnection->SetID(poSock->GetSockId());

		poConnection->SetRemoteIP(pstPerIoData->stRemoteAddr.sin_addr.s_addr);
		poConnection->SetRemotePort(ntohs(pstPerIoData->stRemoteAddr.sin_port));

		FxSession* poSession = m_poSessionFactory->CreateSession();
		if (NULL == poSession)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, CreateSession failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		FxIoThread* poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSock());
		if (NULL == poIoThreadHandler)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, get iothread failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			return;
		}

		poSession->Init(poConnection);
		poConnection->SetSession(poSession);
		poSock->SetIoThread(poIoThreadHandler);

		poSock->SetState(SSTATE_ESTABLISH);

		for (unsigned char i = poSock->m_oRecvWindow.m_btBegin; i != poSock->m_oRecvWindow.m_btEnd; i++)
		{
			unsigned char id = i % poSock->m_oRecvWindow.s_dwWindowSize;
			poSock->m_oRecvWindow.m_pSeqBufferId[id] = poSock->m_oRecvWindow.s_dwWindowSize;
			poSock->m_oRecvWindow.m_pSeqSize[id] = 0;
			poSock->m_oRecvWindow.m_pSeqTime[id] = 0;
			poSock->m_oRecvWindow.m_pSeqRetry[id] = 0;
			poSock->m_oRecvWindow.m_pSeqRetryCount[id] = 0;
		}

		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)pstPerIoData->stWsaBuf.buf;

		unsigned char byRecvBufferId = poSock->m_oRecvWindow.m_btFreeBufferId;
		unsigned char * pRecvBuffer = poSock->m_oRecvWindow.m_ppBuffer[byRecvBufferId];
		poSock->m_oRecvWindow.m_btFreeBufferId = pRecvBuffer[0];

		memcpy(pRecvBuffer, pstPerIoData->stWsaBuf.buf, pstPerIoData->stWsaBuf.len);

		// packet is valid
		if (poSock->m_oRecvWindow.IsValidIndex(pUDPPacketHeader->m_cSyn))
		{
			unsigned char btId = pUDPPacketHeader->m_cSyn % poSock->m_oRecvWindow.s_dwWindowSize;

			if (poSock->m_oRecvWindow.m_pSeqBufferId[btId] >= poSock->m_oRecvWindow.s_dwWindowSize)
			{
				poSock->m_oRecvWindow.m_pSeqBufferId[btId] = byRecvBufferId;
				poSock->m_oRecvWindow.m_pSeqSize[btId] = sizeof(UDPPacketHeader);
			}
		}
		else
		{
			// free buffer.
			pRecvBuffer[0] = poSock->m_oRecvWindow.m_btFreeBufferId;
			poSock->m_oRecvWindow.m_btFreeBufferId = byRecvBufferId;
		}

		// record ack last
		poSock->m_btAckLast = poSock->m_oSendWindow.m_btBegin - 1;

		unsigned char btNewAck = poSock->m_oSendWindow.m_btBegin - 1;
		// calculate new ack
		for (unsigned char i = poSock->m_oRecvWindow.m_btBegin; i != poSock->m_oRecvWindow.m_btEnd; i++)
		{
			// recv buffer is invalid
			if (poSock->m_oRecvWindow.m_pSeqBufferId[i % poSock->m_oRecvWindow.s_dwWindowSize] >= poSock->m_oRecvWindow.s_dwWindowSize)
				break;

			btNewAck = i;
		}

		while (poSock->m_oRecvWindow.m_btBegin != (unsigned char)(btNewAck + 1))
		{
			const unsigned char btHeadSize = sizeof(UDPPacketHeader);
			unsigned char btId = poSock->m_oRecvWindow.m_btBegin % poSock->m_oRecvWindow.s_dwWindowSize;
			unsigned char byBufferId = poSock->m_oRecvWindow.m_pSeqBufferId[btId];
			unsigned char * pBuffer = poSock->m_oRecvWindow.m_ppBuffer[byBufferId] + btHeadSize;

			// copy buffer

			// free buffer
			poSock->m_oRecvWindow.m_ppBuffer[byBufferId][0] = poSock->m_oRecvWindow.m_btFreeBufferId;
			poSock->m_oRecvWindow.m_btFreeBufferId = byBufferId;

			// remove sequence
			poSock->m_oRecvWindow.m_pSeqSize[btId] = 0;
			poSock->m_oRecvWindow.m_pSeqBufferId[btId] = poSock->m_oRecvWindow.s_dwWindowSize;
			poSock->m_oRecvWindow.m_btBegin++;
			poSock->m_oRecvWindow.m_btEnd++;
		}

		while (poSock->m_oSendWindow.m_btBegin != (unsigned char)(pUDPPacketHeader->m_cAck + 1))
		{
			unsigned char id = poSock->m_oSendWindow.m_btBegin % poSock->m_oSendWindow.s_dwWindowSize;
			unsigned char buffer_id = poSock->m_oSendWindow.m_pSeqBufferId[id];

			// free buffer
			poSock->m_oSendWindow.m_ppBuffer[buffer_id][0] = poSock->m_oSendWindow.m_btFreeBufferId;
			poSock->m_oSendWindow.m_btFreeBufferId = buffer_id;
			poSock->m_oSendWindow.m_btBegin++;
		}

		// 这个时候不能说是已经establish 了 要发个消息确认下
		unsigned char btId = poSock->m_oSendWindow.m_btEnd % poSock->m_oSendWindow.s_dwWindowSize;

		// allocate buffer
		unsigned char btBufferId = poSock->m_oSendWindow.m_btFreeBufferId;
		poSock->m_oSendWindow.m_btFreeBufferId = poSock->m_oSendWindow.m_ppBuffer[btBufferId][0];

		// send window buffer
		unsigned char * pBuffer = poSock->m_oSendWindow.m_ppBuffer[btBufferId];

		UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;
		refPacket.m_cStatus = poSock->GetState();
		refPacket.m_cSyn = poSock->m_oSendWindow.m_btEnd;
		refPacket.m_cAck = poSock->m_oRecvWindow.m_btBegin - 1;

		// add to send window
		poSock->m_oSendWindow.m_pSeqBufferId[btId] = btBufferId;
		poSock->m_oSendWindow.m_pSeqSize[btId] = sizeof(UDPPacketHeader);
		poSock->m_oSendWindow.m_pSeqTime[btId] = GetTimeHandler()->GetMilliSecond();
		poSock->m_oSendWindow.m_pSeqRetry[btId] = GetTimeHandler()->GetMilliSecond();
		poSock->m_oSendWindow.m_pSeqRetryTime[btId] = poSock->m_dRetryTime;
		poSock->m_oSendWindow.m_pSeqRetryCount[btId] = 0;
		poSock->m_oSendWindow.m_btEnd++;

		// send的时候 可能要修改 因为 udp tcp 有区别
		if (sendto(poSock->GetSock(), (char*)(&refPacket),
			sizeof(UDPPacketHeader), 0, (sockaddr*)(&pstPerIoData->stRemoteAddr),
			sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "sendto errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				PostAccept(*pstPerIoData);
				poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
				poSock->Close();
				return;
			}
		}

		poSock->m_bSendAck = true;

		sockaddr_in stLocalAddr;
		int nLocalAddrLen = sizeof(sockaddr_in);

		if (getsockname(poSock->GetSock(), (sockaddr*)(&stLocalAddr), &nLocalAddrLen) == SOCKET_ERROR)
		{
			int dwErr = WSAGetLastError();
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "getsockname error: %d", dwErr);

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		poConnection->SetLocalIP(stLocalAddr.sin_addr.s_addr);
		poConnection->SetLocalPort(ntohs(stLocalAddr.sin_port));

		poSock->SetRemoteAddr(pstPerIoData->stRemoteAddr);

		if (false == poSock->AddEvent())
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "poSock->AddEvent failed");

			poSock->Close();
		}
		poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

		if (connect(poSock->GetSock(), (sockaddr*)(&pstPerIoData->stRemoteAddr), sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				PostAccept(*pstPerIoData);
				poSock->PushNetEvent(NETEVT_ERROR, WSAGetLastError());
				poSock->Close();
				return;
			}
		}

		if (false == poSock->PostRecv())
		{
			int dwErr = WSAGetLastError();
			poSock->PushNetEvent(NETEVT_ERROR, dwErr);
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "poSock->PostRecv failed, errno : %d", dwErr);

			poSock->Close();
		}

		PostAccept(*pstPerIoData);

		poSock->PostSendFree();
	}
}

#else
void FxUDPListenSock::OnAccept()
{
	UDPPacketHeader oUDPPacketHeader;

	sockaddr_in stRemoteAddr = { 0 };
	unsigned int nRemoteAddrLen = sizeof(stRemoteAddr);

	if (recvfrom(GetSock(), (char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader), 0, (sockaddr*)&stRemoteAddr, &nRemoteAddrLen))
	{
		if (oUDPPacketHeader.m_cAck != 0)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "ack error want : 0, recv : %d", oUDPPacketHeader.m_cAck);
			return;
		}
		if (oUDPPacketHeader.m_cSyn != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "syn error want : 1, recv : %d", oUDPPacketHeader.m_cSyn);
			return;
		}
		if (oUDPPacketHeader.m_cStatus != SSTATE_CONNECT)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "statu error want : SSTATE_CONNECT, recv : %d", oUDPPacketHeader.m_cStatus);
			return;
		}
	}

	SOCKET hAcceptSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == hAcceptSock)
	{
		return;
	}

	FxUDPConnectSock* poSock = FxMySockMgr::Instance()->CreateUdpSock();
	if (NULL == poSock)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, create CCPSock failed");

		close(hAcceptSock);
		return;
	}

	FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
	if (NULL == poConnection)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, create Connection failed");

		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		return;
	}

	poSock->SetSock(hAcceptSock);
	poSock->SetConnection(poConnection);

	poConnection->SetSockType(SLT_Udp);
	poConnection->SetSock(poSock);
	poConnection->SetID(poSock->GetSockId());

	poConnection->SetRemoteIP(stRemoteAddr.sin_addr.s_addr);
	poConnection->SetRemotePort(ntohs(stRemoteAddr.sin_port));

	FxSession* poSession = m_poSessionFactory->CreateSession();
	if (NULL == poSession)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, CreateSession failed");

		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		FxConnectionMgr::Instance()->Release(poConnection);
		return;
	}

	poSession->Init(poConnection);
	poConnection->SetSession(poSession);

	FxIoThread* poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSock());
	if (NULL == poIoThreadHandler)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "CCPSock::OnAccept, get iothread failed");

		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		return;
	}
	poSock->SetIoThread(poIoThreadHandler);

	setsockopt(poSock->GetSock(), SOL_SOCKET, SO_SNDLOWAT, &UDP_VAL_SO_SNDLOWAT, sizeof(UDP_VAL_SO_SNDLOWAT));
	setsockopt(poSock->GetSock(), SOL_SOCKET, SO_SNDBUF, &UDP_MAX_SYS_SEND_BUF, sizeof(UDP_MAX_SYS_SEND_BUF));

	int nFlags = fcntl(poSock->GetSock(), F_GETFL, 0);
	nFlags |= O_NONBLOCK;
	fcntl(poSock->GetSock(), F_SETFL, nFlags);

	poSock->SetState(SSTATE_ESTABLISH);

	for (unsigned char i = poSock->m_oRecvWindow.m_btBegin; i != poSock->m_oRecvWindow.m_btEnd; i++)
	{
		unsigned char id = i % poSock->m_oRecvWindow.s_dwWindowSize;
		poSock->m_oRecvWindow.m_pSeqBufferId[id] = poSock->m_oRecvWindow.s_dwWindowSize;
		poSock->m_oRecvWindow.m_pSeqSize[id] = 0;
		poSock->m_oRecvWindow.m_pSeqTime[id] = 0;
		poSock->m_oRecvWindow.m_pSeqRetry[id] = 0;
		poSock->m_oRecvWindow.m_pSeqRetryCount[id] = 0;
	}

	UDPPacketHeader* pUDPPacketHeader = &oUDPPacketHeader;

	unsigned char btRecvBufferId = poSock->m_oRecvWindow.m_btFreeBufferId;
	unsigned char * pRecvBuffer = poSock->m_oRecvWindow.m_ppBuffer[btRecvBufferId];
	poSock->m_oRecvWindow.m_btFreeBufferId = pRecvBuffer[0];

	memcpy(pRecvBuffer, (char*)&oUDPPacketHeader, sizeof(oUDPPacketHeader));

	// packet is valid
	if (poSock->m_oRecvWindow.IsValidIndex(pUDPPacketHeader->m_cSyn))
	{
		unsigned char id = pUDPPacketHeader->m_cSyn % poSock->m_oRecvWindow.s_dwWindowSize;

		if (poSock->m_oRecvWindow.m_pSeqBufferId[id] >= poSock->m_oRecvWindow.s_dwWindowSize)
		{
			poSock->m_oRecvWindow.m_pSeqBufferId[id] = btRecvBufferId;
			poSock->m_oRecvWindow.m_pSeqSize[id] = sizeof(UDPPacketHeader);
		}
	}
	else
	{
		// free buffer.
		pRecvBuffer[0] = poSock->m_oRecvWindow.m_btFreeBufferId;
		poSock->m_oRecvWindow.m_btFreeBufferId = btRecvBufferId;
	}

	// record ack last
	poSock->m_btAckLast = poSock->m_oSendWindow.m_btBegin - 1;

	unsigned char btNewAck = poSock->m_oSendWindow.m_btBegin - 1;
	// calculate new ack
	for (unsigned char i = poSock->m_oRecvWindow.m_btBegin; i != poSock->m_oRecvWindow.m_btEnd; i++)
	{
		// recv buffer is invalid
		if (poSock->m_oRecvWindow.m_pSeqBufferId[i % poSock->m_oRecvWindow.s_dwWindowSize] >= poSock->m_oRecvWindow.s_dwWindowSize)
			break;

		btNewAck = i;
	}

	while (poSock->m_oRecvWindow.m_btBegin != (unsigned char)(btNewAck + 1))
	{
		//const unsigned char btHeadSize = sizeof(UDPPacketHeader);
		unsigned char btId = poSock->m_oRecvWindow.m_btBegin % poSock->m_oRecvWindow.s_dwWindowSize;
		unsigned char btBufferId = poSock->m_oRecvWindow.m_pSeqBufferId[btId];
		//unsigned char * pBuffer = poSock->m_oRecvWindow.m_ppBuffer[btBufferId] + btHeadSize;

		// copy buffer

		// free buffer
		poSock->m_oRecvWindow.m_ppBuffer[btBufferId][0] = poSock->m_oRecvWindow.m_btFreeBufferId;
		poSock->m_oRecvWindow.m_btFreeBufferId = btBufferId;

		// remove sequence
		poSock->m_oRecvWindow.m_pSeqSize[btId] = 0;
		poSock->m_oRecvWindow.m_pSeqBufferId[btId] = poSock->m_oRecvWindow.s_dwWindowSize;
		poSock->m_oRecvWindow.m_btBegin++;
		poSock->m_oRecvWindow.m_btEnd++;
	}

	while (poSock->m_oSendWindow.m_btBegin != (unsigned char)(pUDPPacketHeader->m_cAck + 1))
	{
		unsigned char btId = poSock->m_oSendWindow.m_btBegin % poSock->m_oSendWindow.s_dwWindowSize;
		unsigned char btBufferId = poSock->m_oSendWindow.m_pSeqBufferId[btId];

		// free buffer
		poSock->m_oSendWindow.m_ppBuffer[btBufferId][0] = poSock->m_oSendWindow.m_btFreeBufferId;
		poSock->m_oSendWindow.m_btFreeBufferId = btBufferId;
		poSock->m_oSendWindow.m_btBegin++;
	}

	// 这个时候不能说是已经establish 了 要发个消息确认下
	unsigned char btId = poSock->m_oSendWindow.m_btEnd % poSock->m_oSendWindow.s_dwWindowSize;

	// allocate buffer
	unsigned char btBufferId = poSock->m_oSendWindow.m_btFreeBufferId;
	poSock->m_oSendWindow.m_btFreeBufferId = poSock->m_oSendWindow.m_ppBuffer[btBufferId][0];

	// send window buffer
	unsigned char * pBuffer = poSock->m_oSendWindow.m_ppBuffer[btBufferId];

	UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;
	refPacket.m_cStatus = poSock->GetState();
	refPacket.m_cSyn = poSock->m_oSendWindow.m_btEnd;
	refPacket.m_cAck = poSock->m_oRecvWindow.m_btBegin - 1;

	// add to send window
	poSock->m_oSendWindow.m_pSeqBufferId[btId] = btBufferId;
	poSock->m_oSendWindow.m_pSeqSize[btId] = sizeof(UDPPacketHeader);
	poSock->m_oSendWindow.m_pSeqTime[btId] = GetTimeHandler()->GetMilliSecond();
	poSock->m_oSendWindow.m_pSeqRetry[btId] = GetTimeHandler()->GetMilliSecond();
	poSock->m_oSendWindow.m_pSeqRetryTime[btId] = poSock->m_dRetryTime;
	poSock->m_oSendWindow.m_pSeqRetryCount[btId] = 0;
	poSock->m_oSendWindow.m_btEnd++;

	int nReuse = 1;
	setsockopt(poSock->GetSock(), SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof(nReuse));
	if (bind(poSock->GetSock(), (sockaddr*)&m_stAddr, sizeof(m_stAddr)) < 0)
	{
#ifdef _WIN32
		int dwErr = WSAGetLastError();
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "bind failed, errno %d", dwErr);
#else
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "bind failed, errno %d", errno);
#endif // _WIN32
		return;
	}

	// 这个时候不能说是已经establish 了 要发个消息确认下
	// send的时候 可能要修改 因为 udp tcp 有区别
	if (sendto(poSock->GetSock(), (char*)(&refPacket),
		sizeof(UDPPacketHeader), 0, (sockaddr*)(&stRemoteAddr),
		sizeof(stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "sendto errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			poSock->PushNetEvent(NETEVT_ERROR, errno);
			poSock->Close();
			return;
		}
	}
	//poSock->Send((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));

	// // free buffer
	// poSock->m_oSendWindow.m_ppBuffer[btBufferId][0] = poSock->m_oSendWindow.m_btFreeBufferId;
	// poSock->m_oSendWindow.m_btFreeBufferId = btBufferId;
	// ++poSock->m_oSendWindow.m_btBegin;

	poSock->m_bSendAck = true;

	sockaddr_in stLocalAddr = { 0 };
	unsigned int nLocalAddrLen = sizeof(stLocalAddr);
	if (getsockname(poSock->GetSock(), (sockaddr*)&stLocalAddr, &nLocalAddrLen) < 0)
	{
		close(hAcceptSock);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "socket getsockname error : %d, socket : %d, socket id %d", errno, GetSock(), GetSockId());
		poSock->PushNetEvent(NETEVT_ERROR, errno);
		poSock->Close();
		return;
	}

	poConnection->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	poConnection->SetLocalPort(ntohs(stLocalAddr.sin_port));

	poSock->SetRemoteAddr(stRemoteAddr);

	if (false == poSock->AddEvent())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "poSock->AddEvent failed");

		poSock->Close();
	}
	poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

	if (connect(poSock->GetSock(), (sockaddr*)(&stRemoteAddr), sizeof(stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "connect errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			poSock->PushNetEvent(NETEVT_ERROR, errno);
			poSock->Close();
			return;
		}
	}
	return;
}

#endif // _WIN32

FxUDPConnectSock::FxUDPConnectSock()
{
	m_poSendBuf = NULL;
	m_poRecvBuf = NULL;
	m_poConnection = NULL;
	SetState(SSTATE_INVALID);
	SetSock(INVALID_SOCKET);
	m_nNeedData = 0;
	m_nPacketLen = 0;
#ifdef _WIN32
	m_dwLastError = 0;
	m_stRecvIoData.nOp = IOCP_RECV;
	m_stSendIoData.nOp = IOCP_SEND;
	m_nPostRecv = 0;
	m_dwLastError = 0;
#else
#endif // _WIN32

	m_bSendLinger = false;     // 发送延迟，直到成功，或者30次后，这时默认设置//

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

	m_cDelay = 0;

#ifdef _WIN32
	memset(&m_stRecvIoData.stRemoteAddr, 0, sizeof(m_stRecvIoData.stRemoteAddr));
	memset(&m_stSendIoData.stRemoteAddr, 0, sizeof(m_stSendIoData.stRemoteAddr));

	m_byRecvBufferId = 0;
#endif // _WIN32
	if (NULL == m_poSendBuf)
	{
		m_poSendBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poSendBuf)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "NULL == m_poSendBuf, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
	}

	if (NULL == m_poRecvBuf)
	{
		m_poRecvBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poRecvBuf)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "NULL == m_poRecvBuf, socket : %d, socket id : %d", GetSock(), GetSockId());

			return false;
		}
	}

	if (!m_poRecvBuf->Init(RECV_BUFF_SIZE))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "m_poRecvBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (!m_poSendBuf->Init(SEND_BUFF_SIZE))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "m_poSendBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	m_dSendFrequency = 0.02;
	m_dSendDataFrequency = 1;

	m_dDelayTime = 0;
	m_dDelayAverage = 3 * m_dSendFrequency;
	m_dRetryTime = m_dDelayTime + 2 * m_dDelayAverage;
	m_dSendTime = 0;
	m_dAckRecvTime = GetTimeHandler()->GetMilliSecond();
	m_dAckTimeoutRetry = 3;
	m_dwAckSameCount = 0;
	m_bQuickRetry = false;
	m_dSendDataTime = 0;

	m_btAckLast = 0;
	m_btSynLast = 0;
	m_bSendAck = false;
	m_dSendWindowControl = 1;
	m_dSendWindowThreshhold = m_oSendWindow.s_dwWindowSize;

	// clear sliding window buffer
	m_oRecvWindow.ClearBuffer();
	m_oSendWindow.ClearBuffer();

	// initialize send window
	m_oSendWindow.m_btBegin = 1;
	m_oSendWindow.m_btEnd = m_oSendWindow.m_btBegin;

	// initialize recv window
	m_oRecvWindow.m_btBegin = 1;
	m_oRecvWindow.m_btEnd = m_oRecvWindow.m_btBegin + m_oRecvWindow.s_dwWindowSize;

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

#ifdef _WIN32
	m_dwLastError = 0;
	m_nPostRecv = 0;
	m_dwLastError = 0;
#else
#endif // _WIN32
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

#ifdef _WIN32
	shutdown(GetSock(), SD_RECEIVE);
#else
	shutdown(GetSock(), SHUT_RD);
	m_poIoThreadHandler->DelEvent(GetSock());
	// 有bug 就不发了 修改后再说//
	//SendImmediately();
#endif	//WIN32

#ifdef _WIN32
	if (0 != m_dwLastError)
	{
		PushNetEvent(NETEVT_ERROR, m_dwLastError);
		m_dwLastError = 0;
	}
#endif // _WIN32

#ifdef _WIN32
	CancelIo((HANDLE)GetSock());
	closesocket(GetSock());
#else
	close(GetSock());
#endif // _WIN32

	SetSock(INVALID_SOCKET);

	PushNetEvent(NETEVT_TERMINATE, 0);
	m_oLock.UnLock();

	return true;
}

bool FxUDPConnectSock::Send(const char* pData, int dwLen)
{
	if (false == IsConnected())
	{
		LogExe(LogLv_Error, "socket not connected, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (m_poSendBuf->IsEmpty())
	{
		m_poSendBuf->Clear();
	}

	IFxDataHeader* pDataHeader = GetDataHeader();
	if (pDataHeader == NULL)
	{
#ifdef _WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogExe(LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogExe(LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#endif // _WIN32
		return false;
	}

	if ((unsigned int)dwLen + pDataHeader->GetHeaderLength() > (unsigned int)m_poSendBuf->GetTotalLen())
	{
#ifdef _WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogExe(LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogExe(LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#endif // _WIN32
		return false;
	}

	// 这个是在主线程调用 所以 声明为静态就可以了 防止重复生成 占用空间
	static char pTemData[RECV_BUFF_SIZE] = { 0 };

	CNetStream oNetStream(ENetStreamType_Write, pTemData, dwLen + pDataHeader->GetHeaderLength());
	unsigned int dwHeaderLen = 0;
	char* pDataHeaderBuff = (char*)(pDataHeader->BuildSendPkgHeader(dwHeaderLen, dwLen));
	oNetStream.WriteData(pDataHeaderBuff, dwHeaderLen);
	//oNetStream.WriteData((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));
	oNetStream.WriteData(pData, dwLen);

	int nSendCount = 0;
	while (!m_poSendBuf->PushBuff(pTemData, dwLen + dwHeaderLen))
	{
		if (!m_bSendLinger || 30 < ++nSendCount)  // 连续30次还没发出去，就认为失败，失败结果逻辑层处理//
		{
			LogExe(LogLv_Critical, "send buffer overflow!!!!!!!!, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
		FxSleep(10);
	}
	return true;
}

bool FxUDPConnectSock::PushNetEvent(ENetEvtType eType, unsigned int dwValue)
{
	SNetEvent oEvent;
	// 先扔网络事件进去，然后在报告上层有事件，先后顺序不能错，这样上层就不会错取事件//
	oEvent.eType = eType;
	oEvent.dwValue = dwValue;

	while (!FxNetModule::Instance()->PushNetEvent(this, oEvent))
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
#ifdef _WIN32
	if (!m_poIoThreadHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		Close();
		return false;
	}
#else
	if (!m_poIoThreadHandler->AddEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "error : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		Close();
		return false;
	}
#endif // _WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;
}

void FxUDPConnectSock::ProcEvent(SNetEvent oEvent)
{
	switch (oEvent.eType)
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
		__ProcConnectError(oEvent.dwValue);
	}
	break;

	case NETEVT_ERROR:
	{
		__ProcError(oEvent.dwValue);
	}
	break;

	case NETEVT_TERMINATE:
	{
		__ProcTerminate();
	}
	break;

	case NETEVT_RECV:
	{
		__ProcRecv(oEvent.dwValue);
	}
	break;

	case NETEVT_RECV_PACKAGE_ERROR:
	{
		__ProcRecvPackageError(oEvent.dwValue);
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
#ifdef _WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // _WIN32
		LogExe(LogLv_Critical, "create socket failed, errno %d", dwErr);
		return INVALID_SOCKET;
	}

#ifdef _WIN32
	int nSendBuffSize = 256 * 1024;
	int nRecvBuffSize = 8 * nSendBuffSize;
	if ((0 != setsockopt(GetSock(), SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuffSize, sizeof(int))) &&
		(0 != setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuffSize, sizeof(int))))
	{
		LogExe(LogLv_Critical, "setsockopt failed, errno %d", WSAGetLastError());
		closesocket(GetSock());
		return INVALID_SOCKET;
	}

#else
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDLOWAT, &UDP_VAL_SO_SNDLOWAT, sizeof(UDP_VAL_SO_SNDLOWAT));
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, &UDP_MAX_SYS_SEND_BUF, sizeof(UDP_MAX_SYS_SEND_BUF));
#endif // _WIN32

	SetIoThread(FxNetModule::Instance()->FetchIoThread(GetSock()));
	if (NULL == m_poIoThreadHandler)
	{
		LogExe(LogLv_Critical, "%s", "SetIoThread failed");
#ifdef _WIN32
		closesocket(GetSock());
#else
		close(GetSock());
#endif // _WIN32
		return INVALID_SOCKET;
	}

	sockaddr_in stLocalAddr = { 0 };
	stLocalAddr.sin_family = AF_INET;
	stLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stLocalAddr.sin_port = 0;
	if (bind(GetSock(), (sockaddr*)&stLocalAddr, sizeof(stLocalAddr)) < 0)
	{
#ifdef _WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // _WIN32
		LogExe(LogLv_Critical, "connect bind failed, errno %d", dwErr);
		return INVALID_SOCKET;
	}

#ifdef _WIN32
#else
	unsigned
#endif // _WIN32
		int nLocalAddrLen = sizeof(stLocalAddr);
	if (getsockname(GetSock(), (sockaddr*)&stLocalAddr, &nLocalAddrLen) < 0)
	{
#ifdef _WIN32
		closesocket(GetSock());
		int dwErr = WSAGetLastError();
#else
		close(GetSock());
		int dwErr = errno;
#endif // _WIN32

		LogExe(LogLv_Critical, "socket getsockname error : %d, socket : %d, socket id %d", dwErr, GetSock(), GetSockId());
		return INVALID_SOCKET;
	}

	GetConnection()->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	GetConnection()->SetLocalPort(ntohs(stLocalAddr.sin_port));

	sockaddr_in stAddr = { 0 };
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = GetConnection()->GetRemoteIP();
	stAddr.sin_port = htons(GetConnection()->GetRemotePort());

	SetState(SSTATE_CONNECT);

	unsigned char btId = m_oSendWindow.m_btEnd % m_oSendWindow.s_dwWindowSize;

	// allocate buffer
	unsigned char btBufferId = m_oSendWindow.m_btFreeBufferId;
	m_oSendWindow.m_btFreeBufferId = m_oSendWindow.m_ppBuffer[btBufferId][0];

	// send window buffer
	//unsigned char * pBuffer = m_oSendWindow.m_ppBuffer[btBufferId];

	UDPPacketHeader oUDPPacketHeader;
	oUDPPacketHeader.m_cSyn = m_oSendWindow.m_btEnd;
	oUDPPacketHeader.m_cAck = m_oRecvWindow.m_btBegin - 1;
	oUDPPacketHeader.m_cStatus = GetState();

	// add to send window
	m_oSendWindow.m_pSeqBufferId[btId] = btBufferId;
	m_oSendWindow.m_pSeqSize[btId] = sizeof(oUDPPacketHeader);
	m_oSendWindow.m_pSeqTime[btId] = GetTimeHandler()->GetMilliSecond();
	m_oSendWindow.m_pSeqRetry[btId] = GetTimeHandler()->GetMilliSecond();
	m_oSendWindow.m_pSeqRetryTime[btId] = m_dRetryTime;
	m_oSendWindow.m_pSeqRetryCount[btId] = 0;
	m_oSendWindow.m_btEnd++;

	sockaddr_in stRemoteAddr = { 0 };
#ifdef _WIN32
#else
	unsigned
#endif	//WIN32
		int nRemoteAddrLen = sizeof(stRemoteAddr);

	for (unsigned char i = m_oRecvWindow.m_btBegin; i != m_oRecvWindow.m_btEnd; i++)
	{
		unsigned char id = i % m_oRecvWindow.s_dwWindowSize;
		m_oRecvWindow.m_pSeqBufferId[id] = m_oRecvWindow.s_dwWindowSize;
		m_oRecvWindow.m_pSeqSize[id] = 0;
		m_oRecvWindow.m_pSeqTime[id] = 0;
		m_oRecvWindow.m_pSeqRetry[id] = 0;
		m_oRecvWindow.m_pSeqRetryCount[id] = 0;
	}

	unsigned char btRecvBufferId = m_oRecvWindow.m_btFreeBufferId;
	unsigned char * pRecvBuffer = m_oRecvWindow.m_ppBuffer[btRecvBufferId];
	m_oRecvWindow.m_btFreeBufferId = pRecvBuffer[0];

	timeval t1;
	t1.tv_sec = 1;
	t1.tv_usec = 0;
	setsockopt(GetSock(), SOL_SOCKET, SO_RCVTIMEO, (char*)&t1, sizeof(timeval));

ContinuetSend:
	if (sendto(GetSock(), (char*)(&oUDPPacketHeader),
		sizeof(oUDPPacketHeader), 0, (sockaddr*)(&stAddr),
		sizeof(stAddr)) < 0)
	{
#ifdef _WIN32
		int dwErr = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
#else
		int dwErr = errno;
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
#endif // _WIN32
		{
			LogExe(LogLv_Critical, "sendto errno : %d, socket : %d, socket id : %d", dwErr, GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}

	int ret = recvfrom(GetSock(), (char*)(pRecvBuffer), sizeof(UDPPacketHeader), 0, (sockaddr*)&stRemoteAddr, &nRemoteAddrLen);
	if (ret < 0)
	{
		LogExe(LogLv_Error, "recv time out!!!");
		FxSleep(10);
		goto ContinuetSend;
	}

	if (((UDPPacketHeader*)(pRecvBuffer))->m_cAck != 1)
	{
		LogExe(LogLv_Critical, "ack error want : 1, recv : %d", ((UDPPacketHeader*)(pRecvBuffer))->m_cAck);
		return INVALID_SOCKET;
	}
	if (((UDPPacketHeader*)(pRecvBuffer))->m_cSyn != 1)
	{
		LogExe(LogLv_Critical, "syn error want : 1, recv : %d", ((UDPPacketHeader*)(pRecvBuffer))->m_cSyn);
		return INVALID_SOCKET;
	}
	if (((UDPPacketHeader*)(pRecvBuffer))->m_cStatus != SSTATE_ESTABLISH)
	{
		LogExe(LogLv_Critical, "statu error want : SSTATE_ESTABLISH, recv : %d", ((UDPPacketHeader*)(pRecvBuffer))->m_cStatus);
		return INVALID_SOCKET;
	}

	while (m_oSendWindow.m_btBegin != (unsigned char)(oUDPPacketHeader.m_cAck + 1))
	{
		unsigned char btId = m_oSendWindow.m_btBegin % m_oSendWindow.s_dwWindowSize;
		unsigned char btBufferId = m_oSendWindow.m_pSeqBufferId[btId];

		// free buffer
		m_oSendWindow.m_ppBuffer[btBufferId][0] = m_oSendWindow.m_btFreeBufferId;
		m_oSendWindow.m_btFreeBufferId = btBufferId;
		m_oSendWindow.m_btBegin++;
	}

	SetRemoteAddr(stRemoteAddr);

	// packet is valid
	if (m_oRecvWindow.IsValidIndex(((UDPPacketHeader*)(pRecvBuffer))->m_cSyn))
	{
		unsigned char btId = ((UDPPacketHeader*)(pRecvBuffer))->m_cSyn % m_oRecvWindow.s_dwWindowSize;

		if (m_oRecvWindow.m_pSeqBufferId[btId] >= m_oRecvWindow.s_dwWindowSize)
		{
			m_oRecvWindow.m_pSeqBufferId[btId] = btRecvBufferId;
			m_oRecvWindow.m_pSeqSize[btId] = sizeof(UDPPacketHeader);
		}
	}

	//// free buffer.
	//pRecvBuffer[0] = m_oRecvWindow.m_btFreeBufferId;
	//m_oRecvWindow.m_btFreeBufferId = btRecvBufferId;

	// record ack last
	m_btAckLast = m_oSendWindow.m_btBegin - 1;

	unsigned char btNewAck = m_oSendWindow.m_btBegin - 1;
	// calculate new ack
	for (unsigned char i = m_oRecvWindow.m_btBegin; i != m_oRecvWindow.m_btEnd; i++)
	{
		// recv buffer is invalid
		if (m_oRecvWindow.m_pSeqBufferId[i % m_oRecvWindow.s_dwWindowSize] >= m_oRecvWindow.s_dwWindowSize)
			break;

		btNewAck = i;
	}

	while (m_oRecvWindow.m_btBegin != (unsigned char)(btNewAck + 1))
	{
		//const unsigned char btHeadSize = sizeof(UDPPacketHeader);
		unsigned char btId = m_oRecvWindow.m_btBegin % m_oRecvWindow.s_dwWindowSize;
		unsigned char btBufferId = m_oRecvWindow.m_pSeqBufferId[btId];
		//unsigned char * pBuffer = m_oRecvWindow.m_ppBuffer[btBufferId] + btHeadSize;

		// copy buffer

		// free buffer
		m_oRecvWindow.m_ppBuffer[btBufferId][0] = m_oRecvWindow.m_btFreeBufferId;
		m_oRecvWindow.m_btFreeBufferId = btBufferId;

		// remove sequence
		m_oRecvWindow.m_pSeqSize[btId] = 0;
		m_oRecvWindow.m_pSeqBufferId[btId] = m_oRecvWindow.s_dwWindowSize;
		m_oRecvWindow.m_btBegin++;
		m_oRecvWindow.m_btEnd++;
	}

#ifdef _WIN32
	{
		m_stRecvIoData.nOp = IOCP_RECV;
		unsigned long ul = 1;
		if (SOCKET_ERROR == ioctlsocket(GetSock(), FIONBIO, (unsigned long*)&ul))
		{
			PushNetEvent(NETEVT_CONN_ERR, (unsigned int)WSAGetLastError());
			closesocket(GetSock());
			LogExe(LogLv_Critical, "Set socket FIONBIO error : %d, socket : %d, socket id %d", WSAGetLastError(), GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}
#endif // _WIN32

	SetState(SSTATE_ESTABLISH);
	if (!AddEvent())
	{
#ifdef _WIN32
		PushNetEvent(NETEVT_CONN_ERR, WSAGetLastError());
		closesocket(GetSock());
#else
		PushNetEvent(NETEVT_CONN_ERR, errno);
		close(GetSock());
#endif // _WIN32
		return INVALID_SOCKET;
	}

	GetConnection()->SetID(GetSockId());

	PushNetEvent(NETEVT_ESTABLISH, 0);

#ifdef _WIN32
	if (connect(GetSock(), (sockaddr*)(&m_stRemoteAddr), sizeof(m_stRemoteAddr)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			LogExe(LogLv_Critical, "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			return INVALID_SOCKET;
		}
	}

	if (false == PostRecv())
	{
		LogExe(LogLv_Critical, "false == PostRecv(), socket : %d, socket id : %d, errno : %d", GetSock(), GetSockId(), WSAGetLastError());

		return INVALID_SOCKET;
	}

	PostSendFree();
#else
	if (connect(GetSock(), (sockaddr*)(&m_stRemoteAddr), sizeof(m_stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			LogExe(LogLv_Critical, "connect errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			return INVALID_SOCKET;
		}
	}

	int nError = 0;
	socklen_t nLen = sizeof(nError);
	if (getsockopt(GetSock(), SOL_SOCKET, SO_ERROR, &nError, &nLen) < 0)
	{
		LogExe(LogLv_Critical, "getsockopt errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		return INVALID_SOCKET;
	}

	if (nError != 0)
	{
		LogExe(LogLv_Critical, "getsockopt errno : %d, socket : %d, socket id : %d", nError, GetSock(), GetSockId());

		return INVALID_SOCKET;
	}

#endif // _WIN32

	return GetSock();
}

void FxUDPConnectSock::SetRemoteAddr(sockaddr_in& refstRemoteAddr)
{
	memcpy(&m_stRemoteAddr, &refstRemoteAddr, sizeof(refstRemoteAddr));
}

bool FxUDPConnectSock::PostClose()
{
#ifdef _WIN32
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	// Post失败的时候再进入这个函数时可能会丢失一次//

	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), unsigned int(0), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "PostQueuedCompletionStatus errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		return false;
	}

	return true;
#else
	m_poIoThreadHandler->PushDelayCloseSock(this);
	return true;
#endif // _WIN32
}

#ifdef _WIN32
bool FxUDPConnectSock::PostRecv()
{
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

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

	m_byRecvBufferId = m_oRecvWindow.m_btFreeBufferId;
	unsigned char * pBuffer = m_oRecvWindow.m_ppBuffer[m_byRecvBufferId];
	m_oRecvWindow.m_btFreeBufferId = pBuffer[0];

	// can't allocate buffer, disconnect.
	if (m_byRecvBufferId >= m_oRecvWindow.s_dwWindowSize)
	{
		return false;
	}

	m_stRecvIoData.stWsaBuf.buf = (char*)pBuffer;
	m_stRecvIoData.stWsaBuf.len = SlidingWindow::s_dwBufferSize;

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	int nSockAddr = sizeof(m_stRecvIoData.stRemoteAddr);
	if (SOCKET_ERROR == WSARecv(GetSock(), &m_stRecvIoData.stWsaBuf, 1, &dwReadLen, &dwFlags,
		&m_stRecvIoData.stOverlapped, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "WSARecvFrom errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			InterlockedCompareExchange(&m_nPostRecv, 0, 1);
			return false;
		}
	}

	return true;
}

void FxUDPConnectSock::OnParserIoEvent(bool bRet, void* pIoData, unsigned int dwByteTransferred)
{
	SPerUDPIoData* pSPerUDPIoData = (SPerUDPIoData*)pIoData;
	if (NULL == pSPerUDPIoData)
	{
		PushNetEvent(NETEVT_ERROR, NET_CLOSE_ERROR);
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
		default:
		{
			PushNetEvent(NETEVT_ERROR, NET_UNKNOWN_ERROR);
			Close();
		}
		break;
		}
	}
	break;
	default:
	{
		// 如果其他状态收到消息 肯定不对
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "state : %d, error", (unsigned int)GetState());
		PushNetEvent(NETEVT_ERROR, NET_UNKNOWN_ERROR);
		Close();        // 未知错误，不应该发生//
	}
	break;
	}
}
#else
void FxUDPConnectSock::OnParserIoEvent(int dwEvents)
{
	if (!IsConnected() && GetState() != SSTATE_CONNECT)
	{
		ThreadLog(LogLv_Critical, m_poIoThreadHandler->GetFile(), "state error : %d, socket : %d, socket id : %d", GetState(), GetSock(), GetSockId());
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
		if (GetState() == SSTATE_CONNECT)
		{
			return;
		}
		OnRecv();
	}

	if (dwEvents & EPOLLERR)
	{
		if (errno == EINPROGRESS || errno == EINTR || errno == EAGAIN)
		{
			return;
		}
		ThreadLog(LogLv_Critical, m_poIoThreadHandler->GetFile(), "error : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
	}
}

#endif // _WIN32

void FxUDPConnectSock::Update()
{
	if (!PostSend())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == PostSend(), socket : %d, socket id : %d", GetSock(), GetSockId());

		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return;
	}
}

bool FxUDPConnectSock::PostSend()
{
	double dTime = GetTimeHandler()->GetMilliSecond();

	// check ack received time
	if (dTime - m_dAckRecvTime > 5)
	{
		m_dAckRecvTime = dTime;

		if (--m_dAckTimeoutRetry <= 0)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "ack_timeout_retry <= 0, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
	}

	if (dTime < m_dSendTime)
	{
		return true;
	}

	bool bForceRetry = false;

	// enter quick retry when get 3 same ack
	if (m_dwAckSameCount > 3)
	{
		if (m_bQuickRetry == false)
		{
			m_bQuickRetry = true;
			bForceRetry = true;

			m_dSendWindowThreshhold = m_dSendWindowControl / 2;
			if (m_dSendWindowThreshhold < 2) m_dSendWindowThreshhold = 2;
			m_dSendWindowControl = m_dSendWindowThreshhold + m_dwAckSameCount - 1;
			if (m_dSendWindowControl > m_oSendWindow.s_dwWindowSize)
				m_dSendWindowControl = m_oSendWindow.s_dwWindowSize;
		}
		else
		{
			// in quick retry
			// send_window_control increase 1 when get same ack 
			m_dSendWindowControl += 1;
			if (m_dSendWindowControl > m_oSendWindow.s_dwWindowSize)
				m_dSendWindowControl = m_oSendWindow.s_dwWindowSize;
		}
	}
	else
	{
		// quick retry finished when get new ack
		if (m_bQuickRetry == true)
		{
			m_dSendWindowControl = m_dSendWindowThreshhold;
			m_bQuickRetry = false;
		}
	}

	// enter slow start when send data timeout 
	for (unsigned char i = m_oSendWindow.m_btBegin; i != m_oSendWindow.m_btEnd; i++)
	{
		unsigned char btId = i % m_oSendWindow.s_dwWindowSize;
		//unsigned short btSize = m_oSendWindow.m_pSeqSize[btId];

		if (m_oSendWindow.m_pSeqRetryCount[btId] > 0
			&& dTime >= m_oSendWindow.m_pSeqRetry[btId])
		{
			m_dSendWindowThreshhold = m_dSendWindowControl / 2;
			if (m_dSendWindowThreshhold < 2) m_dSendWindowThreshhold = 2;
			//send_window_control = 1;
			m_dSendWindowControl = m_dSendWindowThreshhold;
			//break;

			m_bQuickRetry = false;
			m_dwAckSameCount = 0;
			break;
		}
	}

	unsigned int dwOffset = 0;
	char * pSendBuffer = NULL;
	unsigned int dwSize = m_poSendBuf->GetOutCursorPtr(pSendBuffer);

	// put buffer to send window
	while ((m_oSendWindow.m_btFreeBufferId < m_oSendWindow.s_dwWindowSize) &&	// there is a free buffer
		(dwSize > 0))
	{
		// if send window more than send_window_control, break
		if (m_oSendWindow.m_btEnd - m_oSendWindow.m_btBegin > m_dSendWindowControl)
			break;

		unsigned char btId = m_oSendWindow.m_btEnd % m_oSendWindow.s_dwWindowSize;

		// allocate buffer
		unsigned char btBufferId = m_oSendWindow.m_btFreeBufferId;
		m_oSendWindow.m_btFreeBufferId = m_oSendWindow.m_ppBuffer[btBufferId][0];

		// send window buffer
		unsigned char * pBuffer = m_oSendWindow.m_ppBuffer[btBufferId];

		// packet header
		UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;
		refPacket.m_cStatus = GetState();
		refPacket.m_cSyn = m_oSendWindow.m_btEnd;
		refPacket.m_cAck = m_oRecvWindow.m_btBegin - 1;

		// copy data
		unsigned int dwCopyOffset = sizeof(UDPPacketHeader);
		unsigned int dwCopySize = m_oSendWindow.s_dwBufferSize - dwCopyOffset;
		if (dwCopySize > dwSize)
			dwCopySize = dwSize;

		if (dwCopySize > 0)
		{
			memcpy(pBuffer + dwCopyOffset, pSendBuffer + dwOffset, dwCopySize);

			dwSize -= dwCopySize;
			dwOffset += dwCopySize;
		}

		// add to send window
		m_oSendWindow.m_pSeqBufferId[btId] = btBufferId;
		m_oSendWindow.m_pSeqSize[btId] = dwCopySize + dwCopyOffset;
		m_oSendWindow.m_pSeqTime[btId] = dTime;
		m_oSendWindow.m_pSeqRetry[btId] = dTime;
		m_oSendWindow.m_pSeqRetryTime[btId] = m_dRetryTime;
		m_oSendWindow.m_pSeqRetryCount[btId] = 0;
		m_oSendWindow.m_btEnd++;
	}

	// remove data from send buffer.
	if (dwOffset > 0)
	{
		m_poSendBuf->DiscardBuff(dwOffset);
	}

	// if there is no data to send, make an empty one
	if (m_oSendWindow.m_btBegin == m_oSendWindow.m_btEnd)
	{
		if (dTime >= m_dSendDataTime)
		{
			if (m_oSendWindow.m_btFreeBufferId < m_oSendWindow.s_dwWindowSize)
			{
				unsigned char btId = m_oSendWindow.m_btEnd % m_oSendWindow.s_dwWindowSize;

				// allocate buffer
				unsigned char btBufferId = m_oSendWindow.m_btFreeBufferId;
				m_oSendWindow.m_btFreeBufferId = m_oSendWindow.m_ppBuffer[btBufferId][0];

				// send window buffer
				unsigned char * pBuffer = m_oSendWindow.m_ppBuffer[btBufferId];

				// packet header
				UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;
				refPacket.m_cStatus = GetState();
				refPacket.m_cSyn = m_oSendWindow.m_btEnd;
				refPacket.m_cAck = m_oRecvWindow.m_btBegin - 1;

				// add to send window
				m_oSendWindow.m_pSeqBufferId[btId] = btBufferId;
				m_oSendWindow.m_pSeqSize[btId] = sizeof(UDPPacketHeader);
				m_oSendWindow.m_pSeqTime[btId] = dTime;
				m_oSendWindow.m_pSeqRetry[btId] = dTime;
				m_oSendWindow.m_pSeqRetryTime[btId] = m_dRetryTime;
				m_oSendWindow.m_pSeqRetryCount[btId] = 0;
				m_oSendWindow.m_btEnd++;
			}
		}
	}
	else
		m_dSendDataTime = dTime + m_dSendDataFrequency;

	// send packets
	for (unsigned char i = m_oSendWindow.m_btBegin; i != m_oSendWindow.m_btEnd; i++)
	{
		// if send packets more than send_window_control, break
		if (i - m_oSendWindow.m_btBegin >= m_dSendWindowControl)
			break;

		unsigned char btId = i % m_oSendWindow.s_dwWindowSize;
		unsigned short wSize = m_oSendWindow.m_pSeqSize[btId];

		// send packet
		if (dTime >= m_oSendWindow.m_pSeqRetry[btId] || bForceRetry)
		{
			bForceRetry = false;

			char* pBuffer = (char*)m_oSendWindow.m_ppBuffer[m_oSendWindow.m_pSeqBufferId[btId]];

			// packet header
			UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;
			refPacket.m_cStatus = GetState();
			refPacket.m_cSyn = i;
			refPacket.m_cAck = m_oRecvWindow.m_btBegin - 1;

#ifdef _WIN32
			memset(&m_stSendIoData.stOverlapped, 0, sizeof(m_stSendIoData.stOverlapped));

			m_stSendIoData.stWsaBuf.buf = pBuffer;
			m_stSendIoData.stWsaBuf.len = wSize;
			DWORD dwNumberOfBytesSent = 0;

			int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
				&m_stSendIoData.stOverlapped, NULL);
			if (0 != nRet)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					unsigned int dwErr = WSAGetLastError();
					LogExe(LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

					return false;
				}
			}
#else
			int nRet = send(GetSock(), pBuffer, wSize, 0);
			if (0 > nRet)
			{
				if (EAGAIN == errno)
				{
					return true;
				}

				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "send nRet < 0 err : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

				return false;
			}
			else if (0 == nRet)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "send 0 == nRet, socket : %d, socket id : %d", GetSock(), GetSockId());

				return false;
			}
#endif
			// num send
			num_packets_send++;

			// num retry send
			if (dTime != m_oSendWindow.m_pSeqTime[btId])
				num_packets_retry++;

			m_dSendTime = dTime + m_dSendFrequency;
			m_dSendDataTime = dTime + m_dSendDataFrequency;
			m_bSendAck = false;

			m_oSendWindow.m_pSeqRetryCount[btId]++;
			//send_window.seq_retry_time[id] *= 2;
			m_oSendWindow.m_pSeqRetryTime[btId] = 1.5 * m_dRetryTime;
			if (m_oSendWindow.m_pSeqRetryTime[btId] > 0.2) m_oSendWindow.m_pSeqRetryTime[btId] = 0.2;
			m_oSendWindow.m_pSeqRetry[btId] = dTime + m_oSendWindow.m_pSeqRetryTime[btId];
		}
	}

	// send ack
	if (m_bSendAck)
	{
		UDPPacketHeader oPacket;
		oPacket.m_cStatus = GetState();
		oPacket.m_cSyn = m_oSendWindow.m_btBegin - 1;
		oPacket.m_cAck = m_oRecvWindow.m_btBegin - 1;

#ifdef _WIN32
		memset(&m_stSendIoData.stOverlapped, 0, sizeof(m_stSendIoData.stOverlapped));

		m_stSendIoData.stWsaBuf.buf = (char*)(&oPacket);
		m_stSendIoData.stWsaBuf.len = sizeof(UDPPacketHeader);
		DWORD dwNumberOfBytesSent = 0;

		int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
			&m_stSendIoData.stOverlapped, NULL);
		if (0 != nRet)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				unsigned int dwErr = WSAGetLastError();
				LogExe(LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				return false;
			}
		}
#else
		int nRet = send(GetSock(), (char*)(&oPacket), sizeof(UDPPacketHeader), 0);
		if (0 > nRet)
		{
			if (EAGAIN == errno)
			{
				return true;
			}

			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "send nRet < 0, socket : %d, socket id : %d", GetSock(), GetSockId());

			return false;
		}
		else if (0 == nRet)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "send 0 == nRet, socket : %d, socket id : %d", GetSock(), GetSockId());

			return false;
		}
#endif
		m_dSendTime = dTime + m_dSendFrequency;
		m_bSendAck = false;
	}

	return true;
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
			LogExe(LogLv_Critical, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
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
			LogExe(LogLv_Critical, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnAssociate();
	}
}

void FxUDPConnectSock::__ProcConnectError(unsigned int dwErrorNo)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			LogExe(LogLv_Critical, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnConnError(dwErrorNo);
	}
}

void FxUDPConnectSock::__ProcError(unsigned int dwErrorNo)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			LogExe(LogLv_Critical, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnError(dwErrorNo);
	}
}

void FxUDPConnectSock::__ProcTerminate()
{
	PushNetEvent(NETEVT_RELEASE, 0);
}

void FxUDPConnectSock::__ProcRecv(unsigned int dwLen)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		if (GetConnection()->GetRecvSize() < dwLen)
		{
			LogExe(LogLv_Critical, "session recv_size error need : %d, but : %d", dwLen, GetConnection()->GetRecvSize());
			PushNetEvent(NETEVT_ERROR, NET_RECV_ERROR);
			PostClose();
			return;
		}

		if (!m_poRecvBuf->PopBuff(GetConnection()->GetRecvBuf(), dwLen))
		{
			return;
		}

		memmove(GetConnection()->GetRecvBuf(), GetConnection()->GetRecvBuf() + GetDataHeader()->GetHeaderLength(), dwLen - GetDataHeader()->GetHeaderLength());
		GetConnection()->GetRecvBuf()[dwLen - GetDataHeader()->GetHeaderLength()] = 0;
		GetConnection()->OnRecv(dwLen - GetDataHeader()->GetHeaderLength());
	}
}

void FxUDPConnectSock::__ProcRecvPackageError(unsigned int dwLen)
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			return;
		}

		static char strPopBuf[64 * 1024];
		if (!m_poRecvBuf->PopBuff(strPopBuf, dwLen))
		{
			return;
		}
	}
}

void FxUDPConnectSock::__ProcRelease()
{
	if (GetConnection())
	{
		if (GetSockId() != GetConnection()->GetID())
		{
			LogExe(LogLv_Critical, "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnSocketDestroy();
		GetConnection()->OnClose();

		SetConnection(NULL);
	}
	FxMySockMgr::Instance()->Release(this);
}

#ifdef _WIN32
bool FxUDPConnectSock::PostSendFree()
{
	static SPerUDPIoData oUDPIoData = { 0 };
	oUDPIoData.nOp = IOCP_SEND;
	ZeroMemory(&oUDPIoData.stOverlapped, sizeof(oUDPIoData.stOverlapped));
	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), unsigned int(-1), (ULONG_PTR)this, &oUDPIoData.stOverlapped))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "PostQueuedCompletionStatus error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());
		return false;
	}
	return true;
}

void FxUDPConnectSock::OnRecv(bool bRet, int dwBytes)
{
	if (0 == dwBytes)
	{
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		PushNetEvent(NETEVT_ERROR, NET_EOF_ERROR);
		Close();
		return;
	}

	if (false == bRet)
	{
		int nErr = WSAGetLastError();
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == bRet errno : %d, socket : %d, socket id : %d", nErr, GetSock(), GetSockId());

		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		PushNetEvent(NETEVT_ERROR, nErr);
		Close();
		return;
	}

	if (!IsConnected())
	{
		InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
		return;
	}

	int nUsedLen = 0;
	int nParserLen = 0;

	// packet received
	bool bPacketReceived = false;

	{
		int nLen = int(dwBytes);

		if (nLen < sizeof(UDPPacketHeader))
		{
			PushNetEvent(NETEVT_ERROR, NET_PACKET_ERROR);
			Close();
			return;
		}
		//// allocate buffer
		//unsigned char buffer_id = recv_window.free_buffer_id;
		//unsigned char * buffer = recv_window.buffer[buffer_id];
		//recv_window.free_buffer_id = buffer[0];

		// allocate buffer
		//unsigned char buffer_id = recv_window.free_buffer_id;
		char * pBuffer = m_stRecvIoData.stWsaBuf.buf;
		//recv_window.free_buffer_id = buffer[0];

		//// can't allocate buffer, disconnect.
		//if (buffer_id >= recv_window.window_size)
		//{
		//	PostClose();
		//	return;
		//}

		// receive packet
		int n = nLen;

		// num bytes received
		num_bytes_received += n + 28;

		// packet header
		UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;

		if (refPacket.m_cStatus != SSTATE_ESTABLISH)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv packet state err %d, socket : %d, socket id : %d", refPacket.m_cStatus, GetSock(), GetSockId());
			PushNetEvent(NETEVT_ERROR, NET_CONNECT_FAIL);
			Close();
			return;
		}

		// receive ack, process send buffer.
		if (m_oSendWindow.IsValidIndex(refPacket.m_cAck))
		{
			// got a valid packet
			m_dAckRecvTime = GetTimeHandler()->GetMilliSecond();
			m_dAckTimeoutRetry = 3;

			// static value for calculate delay
			static const double s_dErrFactor = 0.125;
			static const double s_dAverageFactor = 0.25;
			static const double s_dRetryFactor = 2;

			double dRTT = m_dDelayTime;
			double dErrTime = 0;

			// send_window_control not more than double send_window_control 
			double dSendWindowControlMax = m_dSendWindowControl * 2;
			if (dSendWindowControlMax > m_oSendWindow.s_dwWindowSize)
				dSendWindowControlMax = m_oSendWindow.s_dwWindowSize;

			while (m_oSendWindow.m_btBegin != (unsigned char)(refPacket.m_cAck + 1))
			{
				unsigned char btId = m_oSendWindow.m_btBegin % m_oSendWindow.s_dwWindowSize;
				unsigned char btBufferId = m_oSendWindow.m_pSeqBufferId[btId];

				// calculate delay only use no retry packet
				if (m_oSendWindow.m_pSeqRetryCount[btId] == 1)
				{
					// rtt(packet delay)
					dRTT = GetTimeHandler()->GetMilliSecond() - m_oSendWindow.m_pSeqTime[btId];
					// err_time(difference between rtt and delay_time)
					dErrTime = dRTT - m_dDelayTime;
					// revise delay_time with err_time 
					m_dDelayTime = m_dDelayTime + s_dErrFactor * dErrTime;
					// revise delay_average with err_time
					m_dDelayAverage = m_dDelayAverage + s_dAverageFactor * (fabs(dErrTime) - m_dDelayAverage);
				}

				// free buffer
				m_oSendWindow.m_ppBuffer[btBufferId][0] = m_oSendWindow.m_btFreeBufferId;
				m_oSendWindow.m_btFreeBufferId = btBufferId;
				m_oSendWindow.m_btBegin++;

				// get new ack
				// if send_window_control more than send_window_threshhold in congestion avoidance,
				// else in slow start
				// in congestion avoidance send_window_control increase 1
				// in slow start send_window_control increase 1 when get send_window_control count ack
				if (m_dSendWindowControl <= m_dSendWindowThreshhold)
					m_dSendWindowControl += 1;
				else
					m_dSendWindowControl += 1 / m_dSendWindowControl;

				if (m_dSendWindowControl > dSendWindowControlMax)
					m_dSendWindowControl = dSendWindowControlMax;
			}

			// calculate retry with delay_time and delay_average
			m_dRetryTime = m_dDelayTime + s_dRetryFactor * m_dDelayAverage;
			if (m_dRetryTime < m_dSendFrequency) m_dRetryTime = m_dSendFrequency;
		}

		// get same ack
		if (m_btAckLast == m_oSendWindow.m_btBegin - 1)
			m_dwAckSameCount++;
		else
			m_dwAckSameCount = 0;

		// packet is valid
		if (m_oRecvWindow.IsValidIndex(refPacket.m_cSyn))
		{
			unsigned char btId = refPacket.m_cSyn % m_oRecvWindow.s_dwWindowSize;

			if (m_oRecvWindow.m_pSeqBufferId[btId] >= m_oRecvWindow.s_dwWindowSize)
			{
				//recv_window.seq_buffer_id[id] = buffer_id;
				m_oRecvWindow.m_pSeqBufferId[btId] = m_byRecvBufferId;
				m_oRecvWindow.m_pSeqSize[btId] = n;
				bPacketReceived = true;

				//// no more buffer, try parse first.
				//if (recv_window.free_buffer_id >= recv_window.window_size)
				//	break;
				//else
				//	continue;
			}
		}

		if (!bPacketReceived)
		{
			// free buffer.
			pBuffer[0] = m_oRecvWindow.m_btFreeBufferId;
			//recv_window.free_buffer_id = buffer_id;
			m_oRecvWindow.m_btFreeBufferId = m_byRecvBufferId;
		}
	}

	if (m_oSendWindow.m_btBegin == m_oSendWindow.m_btEnd)
		m_dwAckSameCount = 0;

	// record ack last
	m_btAckLast = m_oSendWindow.m_btBegin - 1;

	bool bParseMessage = false;
	// update recv window
	if (bPacketReceived)
	{
		unsigned char btLastAck = m_oRecvWindow.m_btBegin - 1;
		unsigned char btNewAck = btLastAck;

		// calculate new ack
		for (unsigned char i = m_oRecvWindow.m_btBegin; i != m_oRecvWindow.m_btEnd; i++)
		{
			// recv buffer is invalid
			if (m_oRecvWindow.m_pSeqBufferId[i % m_oRecvWindow.s_dwWindowSize] >= m_oRecvWindow.s_dwWindowSize)
				break;

			btNewAck = i;
		}

		// ack changed
		if (btNewAck != btLastAck)
		{
			while (m_oRecvWindow.m_btBegin != (unsigned char)(btNewAck + 1))
			{
				const unsigned char btHeadSize = sizeof(UDPPacketHeader);
				unsigned char btId = m_oRecvWindow.m_btBegin % m_oRecvWindow.s_dwWindowSize;
				unsigned char btBufferId = m_oRecvWindow.m_pSeqBufferId[btId];
				unsigned char * pBuffer = m_oRecvWindow.m_ppBuffer[btBufferId] + btHeadSize;
				unsigned short wSize = m_oRecvWindow.m_pSeqSize[btId] - btHeadSize;

				if (wSize)
				{
					// copy buffer
					// add data to receive buffer
					char* pBuffRecv = NULL;
					int nLenRecv = m_poRecvBuf->GetInCursorPtr(pBuffRecv);
					if (wSize <= nLenRecv)
					{
						m_poRecvBuf->CostBuff(wSize);
						memcpy(pBuffRecv, pBuffer, wSize);
					}
					else
					{
						m_poRecvBuf->CostBuff(nLenRecv);
						memcpy(pBuffRecv, pBuffer, nLenRecv);
						wSize -= nLenRecv;

						//不考虑包超长的情况 只处理包循环放了就可以了
						int n = m_poRecvBuf->GetInCursorPtr(pBuffRecv);
						if (n < wSize)
						{
							ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

							PushNetEvent(NETEVT_ERROR, errno);
							Close();
							return;
						}
						memcpy(pBuffRecv, pBuffer + nLenRecv, wSize);
						m_poRecvBuf->CostBuff(wSize);
					}

					// mark for parse message
					bParseMessage = true;

					// send ack when get packet
					m_bSendAck = true;
				}

				// free buffer
				m_oRecvWindow.m_ppBuffer[btBufferId][0] = m_oRecvWindow.m_btFreeBufferId;
				m_oRecvWindow.m_btFreeBufferId = btBufferId;

				// remove sequence
				m_oRecvWindow.m_pSeqSize[btId] = 0;
				m_oRecvWindow.m_pSeqBufferId[btId] = m_oRecvWindow.s_dwWindowSize;
				m_oRecvWindow.m_btBegin++;
				m_oRecvWindow.m_btEnd++;
			}
		}

		// record receive syn last
		m_btSynLast = m_oRecvWindow.m_btBegin - 1;
	}
	// parse message
	if (bParseMessage)
	{
		char *pUseBuf = NULL;
		int nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
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
				unsigned int dwHeaderLen = GetDataHeader()->GetHeaderLength();
				GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen > nLen ? nLen : dwHeaderLen, 0);
				m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
				if (-1 == m_nPacketLen)
				{
					ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

					InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
					//m_dwLastError = NET_RECVBUFF_ERROR;
					PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
					Close();
					//m_oLock.UnLock();
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
								ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

								InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
								//m_dwLastError = NET_RECVBUFF_ERROR;
								PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
								Close();
								//m_oLock.UnLock();
								return;
							}
							GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
							pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
							m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
							if (0 >= m_nPacketLen)
							{
								ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

								InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
								//m_dwLastError = NET_RECVBUFF_ERROR;
								PushNetEvent(NETEVT_ERROR, NET_RECVBUFF_ERROR);
								Close();
								//m_oLock.UnLock();
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

	InterlockedCompareExchange(&m_nPostRecv, m_nPostRecv - 1, m_nPostRecv);
	if (0 == m_nPostRecv)
	{
		if (false == PostRecv())
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == PostRecv, socket : %d, socket id : %d", GetSock(), GetSockId());

			//m_dwLastError = WSAGetLastError();
			PushNetEvent(NETEVT_ERROR, WSAGetLastError());
			Close();
		}
	}
}

void FxUDPConnectSock::OnSend(bool bRet, int dwBytes)
{
	m_poIoThreadHandler->AddConnectSocket(this);
}

#else
void FxUDPConnectSock::OnRecv()
{
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return;
	}

	if (m_poRecvBuf->IsEmpty())
	{
		m_poRecvBuf->Clear();
	}

	int nUsedLen = 0;
	int nParserLen = 0;
	// packet received
	bool bPacketReceived = false;
	{
		// allocate buffer
		unsigned char btBufferId = m_oRecvWindow.m_btFreeBufferId;
		unsigned char * pBuffer = m_oRecvWindow.m_ppBuffer[btBufferId];
		m_oRecvWindow.m_btFreeBufferId = pBuffer[0];

		// can't allocate buffer, disconnect.
		if (btBufferId >= m_oRecvWindow.s_dwWindowSize)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "can't allocate buffer, socket : %d, socket id : %d", GetSock(), GetSockId());
			PushNetEvent(NETEVT_ERROR, NET_PACKET_ERROR);
			Close();
			return;
		}

		int nLen = recv(GetSock(), pBuffer, m_oRecvWindow.s_dwBufferSize, 0);

		if (0 > nLen)
		{
			if ((errno != EAGAIN) && (errno != EINPROGRESS) && (errno != EINTR))
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

				PushNetEvent(NETEVT_ERROR, errno);
				Close();
				return;
			}
			return;
		}
		if (0 == nLen)
		{
			ThreadLog(LogLv_Debug, m_poIoThreadHandler->GetFile(), "recv len 0");
			PushNetEvent(NETEVT_ERROR, NET_EOF_ERROR);
			Close();
			return;
		}

		if (nLen < (int)sizeof(UDPPacketHeader))
		{
			// free buffer
			pBuffer[0] = m_oRecvWindow.m_btFreeBufferId;
			m_oRecvWindow.m_btFreeBufferId = btBufferId;

			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv packet head err, socket : %d, socket id : %d", GetSock(), GetSockId());
			PushNetEvent(NETEVT_ERROR, NET_PACKET_ERROR);
			Close();
			return;
		}

		// packet header
		UDPPacketHeader & refPacket = *(UDPPacketHeader*)pBuffer;

		if (refPacket.m_cStatus != SSTATE_ESTABLISH)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv packet state err %d, socket : %d, socket id : %d", refPacket.m_cStatus, GetSock(), GetSockId());
			PushNetEvent(NETEVT_ERROR, NET_CONNECT_FAIL);
			Close();
			return;
		}

		// receive ack, process send buffer.
		if (m_oSendWindow.IsValidIndex(refPacket.m_cAck))
		{
			// got a valid packet
			m_dAckRecvTime = GetTimeHandler()->GetMilliSecond();
			m_dAckTimeoutRetry = 3;

			// static value for calculate delay
			static const double s_dErrFactor = 0.125;
			static const double s_dAverageFactor = 0.25;
			static const double s_dRetryFactor = 2;

			double dRTT = m_dDelayTime;
			double dErrTime = 0;

			// send_window_control not more than double send_window_control 
			double dSendWindowControlMax = m_dSendWindowControl * 2;
			if (dSendWindowControlMax > m_oSendWindow.s_dwWindowSize)
				dSendWindowControlMax = m_oSendWindow.s_dwWindowSize;

			while (m_oSendWindow.m_btBegin != (unsigned char)(refPacket.m_cAck + 1))
			{
				unsigned char btId = m_oSendWindow.m_btBegin % m_oSendWindow.s_dwWindowSize;
				unsigned char btBufferId = m_oSendWindow.m_pSeqBufferId[btId];

				// calculate delay only use no retry packet
				if (m_oSendWindow.m_pSeqRetryCount[btId] == 1)
				{
					// rtt(packet delay)
					dRTT = GetTimeHandler()->GetMilliSecond() - m_oSendWindow.m_pSeqTime[btId];
					// err_time(difference between rtt and delay_time)
					dErrTime = dRTT - m_dDelayTime;
					// revise delay_time with err_time 
					m_dDelayTime = m_dDelayTime + s_dErrFactor * dErrTime;
					// revise delay_average with err_time
					m_dDelayAverage = m_dDelayAverage + s_dAverageFactor * (fabs(dErrTime) - m_dDelayAverage);
				}

				// free buffer
				m_oSendWindow.m_ppBuffer[btBufferId][0] = m_oSendWindow.m_btFreeBufferId;
				m_oSendWindow.m_btFreeBufferId = btBufferId;
				m_oSendWindow.m_btBegin++;

				// get new ack
				// if send_window_control more than send_window_threshhold in congestion avoidance,
				// else in slow start
				// in congestion avoidance send_window_control increase 1
				// in slow start send_window_control increase 1 when get send_window_control count ack
				if (m_dSendWindowControl <= m_dSendWindowThreshhold)
					m_dSendWindowControl += 1;
				else
					m_dSendWindowControl += 1 / m_dSendWindowControl;

				if (m_dSendWindowControl > dSendWindowControlMax)
					m_dSendWindowControl = dSendWindowControlMax;
			}

			// calculate retry with delay_time and delay_average
			m_dRetryTime = m_dDelayTime + s_dRetryFactor * m_dDelayAverage;
			if (m_dRetryTime < m_dSendFrequency) m_dRetryTime = m_dSendFrequency;
		}

		// get same ack
		if (m_btAckLast == m_oSendWindow.m_btBegin - 1)
			m_dwAckSameCount++;
		else
			m_dwAckSameCount = 0;

		// packet is valid
		if (m_oRecvWindow.IsValidIndex(refPacket.m_cSyn))
		{
			unsigned char id = refPacket.m_cSyn % m_oRecvWindow.s_dwWindowSize;

			if (m_oRecvWindow.m_pSeqBufferId[id] >= m_oRecvWindow.s_dwWindowSize)
			{
				m_oRecvWindow.m_pSeqBufferId[id] = btBufferId;
				m_oRecvWindow.m_pSeqSize[id] = nLen;
				bPacketReceived = true;

				//// no more buffer, try parse first.
				//if (recv_window.free_buffer_id >= recv_window.window_size)
				//	break;
				//else
				//	continue;
			}
		}
		if (!bPacketReceived)
		{
			// free buffer.
			pBuffer[0] = m_oRecvWindow.m_btFreeBufferId;
			m_oRecvWindow.m_btFreeBufferId = btBufferId;
		}
	}

	if (m_oSendWindow.m_btBegin == m_oSendWindow.m_btEnd)
		m_dwAckSameCount = 0;

	// record ack last
	m_btAckLast = m_oSendWindow.m_btBegin - 1;

	bool bParseMessage = false;
	// update recv window
	if (bPacketReceived)
	{
		unsigned char btLastAck = m_oRecvWindow.m_btBegin - 1;
		unsigned char btNewAck = btLastAck;

		// calculate new ack
		for (unsigned char i = m_oRecvWindow.m_btBegin; i != m_oRecvWindow.m_btEnd; i++)
		{
			// recv buffer is invalid
			if (m_oRecvWindow.m_pSeqBufferId[i % m_oRecvWindow.s_dwWindowSize] >= m_oRecvWindow.s_dwWindowSize)
				break;

			btNewAck = i;
		}

		// ack changed
		if (btNewAck != btLastAck)
		{
			while (m_oRecvWindow.m_btBegin != (unsigned char)(btNewAck + 1))
			{
				const unsigned char btHeadSize = sizeof(UDPPacketHeader);
				unsigned char btId = m_oRecvWindow.m_btBegin % m_oRecvWindow.s_dwWindowSize;
				unsigned char btBufferId = m_oRecvWindow.m_pSeqBufferId[btId];
				unsigned char * pBuffer = m_oRecvWindow.m_ppBuffer[btBufferId] + btHeadSize;
				unsigned short wSize = m_oRecvWindow.m_pSeqSize[btId] - btHeadSize;

				// copy buffer
				// add data to receive buffer
				if (wSize)
				{
					char* pBuffRecv = NULL;
					int nLenRecv = m_poRecvBuf->GetInCursorPtr(pBuffRecv);
					if (wSize <= nLenRecv)
					{
						m_poRecvBuf->CostBuff(wSize);
						memcpy(pBuffRecv, pBuffer, wSize);
					}
					else
					{
						m_poRecvBuf->CostBuff(nLenRecv);
						memcpy(pBuffRecv, pBuffer, nLenRecv);
						wSize -= nLenRecv;

						//不考虑包超长的情况 只处理包循环放了就可以了
						int n = m_poRecvBuf->GetInCursorPtr(pBuffRecv);
						if (n < wSize)
						{
							ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "recv errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

							PushNetEvent(NETEVT_ERROR, errno);
							Close();
							return;
						}
						memcpy(pBuffRecv, pBuffer + nLenRecv, wSize);
						m_poRecvBuf->CostBuff(wSize);
					}

					// mark for parse message
					bParseMessage = true;
				}

				// free buffer
				m_oRecvWindow.m_ppBuffer[btBufferId][0] = m_oRecvWindow.m_btFreeBufferId;
				m_oRecvWindow.m_btFreeBufferId = btBufferId;

				// remove sequence
				m_oRecvWindow.m_pSeqSize[btId] = 0;
				m_oRecvWindow.m_pSeqBufferId[btId] = m_oRecvWindow.s_dwWindowSize;
				m_oRecvWindow.m_btBegin++;
				m_oRecvWindow.m_btEnd++;

				// send ack when get packet
				m_bSendAck = true;
			}
		}

		// record receive syn last
		m_btSynLast = m_oRecvWindow.m_btBegin - 1;
	}
	// parse message
	if (bParseMessage)
	{
		char *pUseBuf = NULL;
		int nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
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
				unsigned int dwHeaderLen = GetDataHeader()->GetHeaderLength();
				GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen > nLen ? nLen : dwHeaderLen, 0);
				m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
				if (-1 == m_nPacketLen)
				{
					ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

					nUsedLen += nLen;
					m_poRecvBuf->CostUsedBuff(nUsedLen);
					nUsedLen = 0;
					PushNetEvent(NETEVT_ERROR, errno);
					Close();
					return;
					nLen = 0;
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
								ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

								nUsedLen += nLen;
								m_poRecvBuf->CostUsedBuff(nUsedLen);
								nUsedLen = 0;
								PushNetEvent(NETEVT_ERROR, errno);
								Close();
								return;
								nLen = 0;
							}
							GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
							pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
							m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
							if (0 >= m_nPacketLen)
							{
								ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

								nUsedLen += nLen;
								m_poRecvBuf->CostUsedBuff(nUsedLen);
								nUsedLen = 0;
								PushNetEvent(NETEVT_ERROR, errno);
								Close();
								return;
								nLen = 0;
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

void FxUDPConnectSock::OnSend()
{
	if (!IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return;
	}

	m_poIoThreadHandler->AddConnectSocket(this);

	if (!m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), "error : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		Close();
		return;
	}
}


#endif // _WIN32

