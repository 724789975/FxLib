#include "myudpsock.h"
#include "sockmgr.h"
#include <stdio.h>
#include "connectionmgr.h"
#include "connection.h"
#include "iothread.h"
#include "net.h"
#include "netstream.h"

#define RECV_BUFF_SIZE 32*64*1024
#define SEND_BUFF_SIZE 64*1024

#define MAX_LOST_PACKET 10

#ifdef WIN32
#else
#include <unistd.h>
int UDP_MAX_SYS_SEND_BUF = (128 * 1024);
int UDP_VAL_SO_SNDLOWAT = (64 * 1024);
#endif // WIN32

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

bool FxUDPListenSock::Listen(UINT32 dwIP, UINT16 wPort)
{
	SetSock(socket(AF_INET, SOCK_DGRAM, 0));
	if (INVALID_SOCKET == GetSock())
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "create socket error, %u:%u, errno %d", dwIP, wPort, dwErr);
#else
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "create socket error, %u:%u, errno %d", dwIP, wPort, errno);
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
		closesocket(GetSock());
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "Set socket FIONBIO error : %d, socket : %d, socket id %d",
			WSAGetLastError(), GetSock(), GetSockId());
		return false;
	}
#endif // WIN32

	if (bind(GetSock(), (sockaddr*)&stAddr, sizeof(stAddr)) < 0)
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "bind at %u:%d failed, errno %d", dwIP, wPort, dwErr);
#else
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "bind at %u:%d failed, errno %d", dwIP, wPort, errno);
#endif // WIN32
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
	ThreadLog(LogLv_Info, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "listen at %u:%d success", dwIP, wPort);

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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "state : %d != SSTATE_LISTEN", (UINT32)GetState());
		return false;
	}

	if (INVALID_SOCKET == GetSock())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "socket : %d == INVALID_SOCKET", GetSock());
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "state : %d == SSTATE_INVALID", (UINT32)GetState());

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "OnParserIoEvent failed, errno %d", dwErr);

			closesocket(pSPerUDPIoData->hSock);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}
		if (dwByteTransferred != sizeof(UDPPacketHeader))
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "%s", "recv length error !!!");
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		UDPPacketHeader* pUDPPacketHeader = (UDPPacketHeader*)pSPerUDPIoData->stWsaBuf.buf;

		if ((ESocketState)pUDPPacketHeader->m_cStatus != SSTATE_CONNECT)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "recv socket state error want : %d, recv : %d !!!",
				SSTATE_CONNECT, pUDPPacketHeader->m_cStatus);
			PostAccept(*pSPerUDPIoData);
			m_oLock.UnLock();
			return;
		}

		if (pUDPPacketHeader->m_cSyn != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "recv socket syn error want : %d, recv : %d !!!",
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "%s", "listen socket has stoped but ret is true");
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "state : %d != SSTATE_LISTEN", (UINT32)GetState());

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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "get error event errno : %d", errno);

		Close();
		return;
	}

	if (dwEvents & EPOLLIN)
	{
		OnAccept();
	}
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "WSASocket failed, errno %d", dwErr);
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "WSARecvFrom errno : %d, socket : %d, socket id : %d",
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "state : %d != SSTATE_LISTEN", GetState());

		closesocket(hSock);
		return;
	}

	{
		FxUDPConnectSock* poSock = FxMySockMgr::Instance()->CreateUdpSock();
		if (NULL == poSock)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, create CCPSock failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			return;
		}

		FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
		if (NULL == poConnection)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, create Connection failed");

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, CreateSession failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			FxConnectionMgr::Instance()->Release(poConnection);
			return;
		}

		FxIoThread* poIoThreadHandler = FxNetModule::Instance()->FetchIoThread(poSock->GetSock());
		if (NULL == poIoThreadHandler)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, get iothread failed");

			closesocket(hSock);
			PostAccept(*pstPerIoData);
			FxMySockMgr::Instance()->Release(poSock);
			return;
		}

		poSession->Init(poConnection);
		poConnection->SetSession(poSession);
		poSock->SetIoThread(poIoThreadHandler);

		poSock->SetState(SSTATE_ESTABLISH);

		for (byte i = poSock->recv_window.begin; i != poSock->recv_window.end; i++)
		{
			byte id = i % poSock->recv_window.window_size;
			poSock->recv_window.seq_buffer_id[id] = poSock->recv_window.window_size;
			poSock->recv_window.seq_size[id] = 0;
			poSock->recv_window.seq_time[id] = 0;
			poSock->recv_window.seq_retry[id] = 0;
			poSock->recv_window.seq_retry_count[id] = 0;
		}



		// 这个时候不能说是已经establish 了 要发个消息确认下
		byte id = poSock->send_window.end % poSock->send_window.window_size;

		// allocate buffer
		byte buffer_id = poSock->send_window.free_buffer_id;
		poSock->send_window.free_buffer_id = poSock->send_window.buffer[buffer_id][0];

		// send window buffer
		byte * buffer = poSock->send_window.buffer[buffer_id];

		UDPPacketHeader & packet = *(UDPPacketHeader*)buffer;
		packet.m_cStatus = poSock->GetState();
		packet.m_cSyn = poSock->send_window.end;
		packet.m_cAck = poSock->recv_window.begin - 1;

		// add to send window
		poSock->send_window.seq_buffer_id[id] = buffer_id;
		poSock->send_window.seq_size[id] = sizeof(packet);
		poSock->send_window.seq_time[id] = GetTimeHandler()->GetMilliSecond();
		poSock->send_window.seq_retry[id] = GetTimeHandler()->GetMilliSecond();
		poSock->send_window.seq_retry_time[id] = poSock->retry_time;
		poSock->send_window.seq_retry_count[id] = 0;
		poSock->send_window.end++;

		++poSock->send_window.begin;

		// send的时候 可能要修改 因为 udp tcp 有区别
		if (sendto(poSock->GetSock(), (char*)(&packet),
			sizeof(packet), 0, (sockaddr*)(&pstPerIoData->stRemoteAddr),
			sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "sendto errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "getsockname error: %d", dwErr);

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "poSock->AddEvent failed");

			poSock->Close();
		}
		poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

		if (connect(poSock->GetSock(), (sockaddr*)(&pstPerIoData->stRemoteAddr), sizeof(pstPerIoData->stRemoteAddr)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

				PostAccept(*pstPerIoData);
				poSock->Close();
				return;
			}
		}

		if (false == poSock->PostRecv())
		{
			int dwErr = WSAGetLastError();
			poSock->PushNetEvent(NETEVT_ERROR, dwErr);
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "poSock->PostRecv failed, errno : %d", dwErr);

			poSock->Close();
		}

		PostAccept(*pstPerIoData);
	}
}

#else
void FxUDPListenSock::OnAccept()
{
	UDPPacketHeader oUDPPacketHeader = { 0 };

	sockaddr_in stRemoteAddr = { 0 };
	unsigned int nRemoteAddrLen = sizeof(stRemoteAddr);

	if (recvfrom(GetSock(), (char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader), 0, (sockaddr*)&stRemoteAddr, &nRemoteAddrLen))
	{
		if (oUDPPacketHeader.m_cAck != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "ack error want : 1, recv : %d", oUDPPacketHeader.m_cAck);
			return;
		}
		if (oUDPPacketHeader.m_cSyn != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "syn error want : 1, recv : %d", oUDPPacketHeader.m_cSyn);
			return;
		}
		if (oUDPPacketHeader.m_cStatus != SSTATE_CONNECT)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "statu error want : SSTATE_CONNECT, recv : %d", oUDPPacketHeader.m_cStatus);
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, create CCPSock failed");

		close(hAcceptSock);
		return;
	}

	FxConnection* poConnection = FxConnectionMgr::Instance()->Create();
	if (NULL == poConnection)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, create Connection failed");

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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, CreateSession failed");

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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "CCPSock::OnAccept, get iothread failed");

		close(hAcceptSock);
		FxMySockMgr::Instance()->Release(poSock);
		return;
	}
	poSock->SetIoThread(poIoThreadHandler);

	setsockopt(poSock->GetSock(), SOL_SOCKET, SO_SNDLOWAT, &UDP_VAL_SO_SNDLOWAT, sizeof(UDP_VAL_SO_SNDLOWAT));
	setsockopt(poSock->GetSock(), SOL_SOCKET, SO_SNDBUF, &UDP_MAX_SYS_SEND_BUF, sizeof(UDP_MAX_SYS_SEND_BUF));

	poSock->SetState(SSTATE_ESTABLISH);

	// 这个时候不能说是已经establish 了 要发个消息确认下
	UDPPacketHeader oSendUDPPacketHeader = { 0 };
	oSendUDPPacketHeader.m_cAck = oUDPPacketHeader.m_cSyn;
	oSendUDPPacketHeader.m_cSyn = 1;
	oSendUDPPacketHeader.m_cStatus = poSock->GetState();

	// send的时候 可能要修改 因为 udp tcp 有区别
	if (sendto(poSock->GetSock(), (char*)(&oSendUDPPacketHeader),
		sizeof(oSendUDPPacketHeader), 0, (sockaddr*)(&stRemoteAddr),
		sizeof(stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "sendto errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			poSock->Close();
			return;
		}
	}
	//poSock->Send((char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader));

	sockaddr_in stLocalAddr = { 0 };
	unsigned int nLocalAddrLen = sizeof(stLocalAddr);
	if (getsockname(poSock->GetSock(), (sockaddr*)&stLocalAddr, &nLocalAddrLen) < 0)
	{
		close(hAcceptSock);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "socket getsockname error : %d, socket : %d, socket id %d", errno, GetSock(), GetSockId());
		poSock->Close();
		return;
	}

	poConnection->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	poConnection->SetLocalPort(ntohs(stLocalAddr.sin_port));

	poSock->SetRemoteAddr(stRemoteAddr);

	if (false == poSock->AddEvent())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "poSock->AddEvent failed");

		poSock->Close();
	}
	poSock->PushNetEvent(NETEVT_ESTABLISH, 0);

	if (connect(poSock->GetSock(), (sockaddr*)(&stRemoteAddr), sizeof(stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "connect errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			poSock->Close();
			return;
		}
	}
	return;
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

#ifdef WIN32
	memset(&m_stRecvIoData.stRemoteAddr, 0, sizeof(m_stRecvIoData.stRemoteAddr));
	memset(&m_stSendIoData.stRemoteAddr, 0, sizeof(m_stSendIoData.stRemoteAddr));
#endif // WIN32
	if (NULL == m_poSendBuf)
	{
		m_poSendBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poSendBuf)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "NULL == m_poSendBuf, socket : %d, socket id : %d", GetSock(), GetSockId());
			return false;
		}
	}

	if (NULL == m_poRecvBuf)
	{
		m_poRecvBuf = FxLoopBuffMgr::Instance()->Fetch();
		if (NULL == m_poRecvBuf)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "NULL == m_poRecvBuf, socket : %d, socket id : %d", GetSock(), GetSockId());

			return false;
		}
	}

	if (!m_poRecvBuf->Init(RECV_BUFF_SIZE))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poRecvBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (!m_poSendBuf->Init(SEND_BUFF_SIZE))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poSendBuf->Init failed, socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	delay_time = 0;
	delay_average = 3 * send_frequency;
	retry_time = delay_time + 2 * delay_average;
	send_time = 0;
	ack_recv_time = GetTimeHandler()->GetMilliSecond();
	ack_timeout_retry = 3;
	ack_same_count = 0;
	quick_retry = false;
	send_data_time = 0;

	ack_last = 0;
	syn_last = 0;
	send_ack = false;
	send_window_control = 1;
	send_window_threshhold = send_window.window_size;

	// clear sliding window buffer
	recv_window.ClearBuffer();
	send_window.ClearBuffer();

	// initialize send window
	send_window.begin = 1;
	send_window.end = send_window.begin;

	// initialize recv window
	recv_window.begin = 1;
	recv_window.end = recv_window.begin + recv_window.window_size;

	return true;
}

void FxUDPConnectSock::OnRead()
{
}

void FxUDPConnectSock::OnWrite()
{
	PostSend();
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
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogExe(LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogExe(LogLv_Error, "send error pDataHeader == NULL, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#endif // WIN32
		return false;
	}

	if ((unsigned int)dwLen + pDataHeader->GetHeaderLength() > (unsigned int)m_poSendBuf->GetTotalLen())
	{
#ifdef WIN32
		m_dwLastError = NET_SEND_OVERFLOW;
		LogExe(LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#else
		PushNetEvent(NETEVT_ERROR, NET_SEND_OVERFLOW);
		LogExe(LogLv_Error, "send error NET_SEND_OVERFLOW, socket : %d, socket id : %d", GetSock(), GetSockId());

		PostClose();
#endif // WIN32
		return false;
	}

	// 这个是在主线程调用 所以 声明为静态就可以了 防止重复生成 占用空间
	static char pTemData[RECV_BUFF_SIZE] = { 0 };

	CNetStream oNetStream(ENetStreamType_Write, pTemData, dwLen + pDataHeader->GetHeaderLength());
	UINT32 dwHeaderLen = 0;
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
	PostSend();
	return true;
}

bool FxUDPConnectSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
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
#ifdef WIN32
	if (!m_poIoThreadHandler->AddEvent(GetSock(), this))
	{
		PushNetEvent(NETEVT_ERROR, WSAGetLastError());
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		Close();
		return false;
	}
#else
	if (!m_poIoThreadHandler->AddEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		PushNetEvent(NETEVT_ERROR, errno);
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "error : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		Close();
		return false;
	}
#endif // WIN32

	PushNetEvent(NETEVT_ASSOCIATE, 0);
	return true;
}

void FxUDPConnectSock::ProcEvent(SNetEvent oEvent)
{
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
#ifdef WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // WIN32
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "create socket failed, errno %d", dwErr);
		return INVALID_SOCKET;
	}

#ifdef WIN32
	int nSendBuffSize = 256 * 1024;
	int nRecvBuffSize = 8 * nSendBuffSize;
	if ((0 != setsockopt(GetSock(), SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuffSize, sizeof(int))) &&
		(0 != setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuffSize, sizeof(int))))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "setsockopt failed, errno %d", WSAGetLastError());
		closesocket(GetSock());
		return INVALID_SOCKET;
	}

#else
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDLOWAT, &UDP_VAL_SO_SNDLOWAT, sizeof(UDP_VAL_SO_SNDLOWAT));
	setsockopt(GetSock(), SOL_SOCKET, SO_SNDBUF, &UDP_MAX_SYS_SEND_BUF, sizeof(UDP_MAX_SYS_SEND_BUF));
#endif // WIN32

	SetIoThread(FxNetModule::Instance()->FetchIoThread(GetSock()));
	if (NULL == m_poIoThreadHandler)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "%s", "SetIoThread failed");
#ifdef WIN32
		closesocket(GetSock());
#else
		close(GetSock());
#endif // WIN32
		return INVALID_SOCKET;
	}

	sockaddr_in stLocalAddr = { 0 };
	stLocalAddr.sin_family = AF_INET;
	stLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stLocalAddr.sin_port = 0;
	if (bind(GetSock(), (sockaddr*)&stLocalAddr, sizeof(stLocalAddr)) < 0)
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
#else
		int dwErr = errno;
#endif // WIN32
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "connect bind failed, errno %d", dwErr);
		return INVALID_SOCKET;
	}

#ifdef WIN32
#else
	unsigned
#endif // WIN32
	int nLocalAddrLen = sizeof(stLocalAddr);
	if (getsockname(GetSock(), (sockaddr*)&stLocalAddr, &nLocalAddrLen) < 0)
	{
#ifdef WIN32
		closesocket(GetSock());
		int dwErr = WSAGetLastError();
#else
		close(GetSock());
		int dwErr = errno;
#endif // WIN32
		
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "socket getsockname error : %d, socket : %d, socket id %d", dwErr, GetSock(), GetSockId());
		return INVALID_SOCKET;
	}

	GetConnection()->SetLocalIP(stLocalAddr.sin_addr.s_addr);
	GetConnection()->SetLocalPort(ntohs(stLocalAddr.sin_port));

	sockaddr_in stAddr = { 0 };
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = GetConnection()->GetRemoteIP();
	stAddr.sin_port = htons(GetConnection()->GetRemotePort());

	SetState(SSTATE_CONNECT);

	byte id = send_window.end % send_window.window_size;

	// allocate buffer
	byte buffer_id = send_window.free_buffer_id;
	send_window.free_buffer_id = send_window.buffer[buffer_id][0];

	// send window buffer
	byte * buffer = send_window.buffer[buffer_id];

	UDPPacketHeader oUDPPacketHeader = { 0 };
	oUDPPacketHeader.m_cSyn = send_window.end;
	oUDPPacketHeader.m_cAck = recv_window.begin - 1;
	oUDPPacketHeader.m_cStatus = GetState();

	// add to send window
	send_window.seq_buffer_id[id] = buffer_id;
	send_window.seq_size[id] = sizeof(oUDPPacketHeader);
	send_window.seq_time[id] = GetTimeHandler()->GetMilliSecond();
	send_window.seq_retry[id] = GetTimeHandler()->GetMilliSecond();
	send_window.seq_retry_time[id] = retry_time;
	send_window.seq_retry_count[id] = 0;
	send_window.end++;

	if (sendto(GetSock(), (char*)(&oUDPPacketHeader),
		sizeof(oUDPPacketHeader), 0, (sockaddr*)(&stAddr),
		sizeof(stAddr)) < 0)
	{
#ifdef WIN32
		int dwErr = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
#else
		int dwErr = errno;
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
#endif // WIN32
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "sendto errno : %d, socket : %d, socket id : %d", dwErr, GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}

	sockaddr_in stRemoteAddr = { 0 };
#ifdef WIN32
#else
	unsigned
#endif	//WIN32
	int nRemoteAddrLen = sizeof(stRemoteAddr);

	oUDPPacketHeader = {0};
	if (recvfrom(GetSock(), (char*)(&oUDPPacketHeader), sizeof(oUDPPacketHeader), 0, (sockaddr*)&stRemoteAddr, &nRemoteAddrLen))
	{
		if (oUDPPacketHeader.m_cAck != 0)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "ack error want : 1, recv : %d", oUDPPacketHeader.m_cAck);
			return INVALID_SOCKET;
		}
		if (oUDPPacketHeader.m_cSyn != 1)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "syn error want : 1, recv : %d", oUDPPacketHeader.m_cSyn);
			return INVALID_SOCKET;
		}
		if (oUDPPacketHeader.m_cStatus != SSTATE_ESTABLISH)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "statu error want : SSTATE_ESTABLISH, recv : %d", oUDPPacketHeader.m_cStatus);
			return INVALID_SOCKET;
		}

		for (unsigned char i = recv_window.begin; i != recv_window.end; i++)
		{
			unsigned char id = i % recv_window.window_size;
			recv_window.seq_buffer_id[id] = recv_window.window_size;
			recv_window.seq_size[id] = 0;
			recv_window.seq_time[id] = 0;
			recv_window.seq_retry[id] = 0;
			recv_window.seq_retry_count[id] = 0;
		}

		++send_window.begin;

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "Set socket FIONBIO error : %d, socket : %d, socket id %d", WSAGetLastError(), GetSock(), GetSockId());
			return INVALID_SOCKET;
		}
	}
#endif // WIN32

	if (!AddEvent())
	{
#ifdef WIN32
		PushNetEvent(NETEVT_CONN_ERR, WSAGetLastError());
		closesocket(GetSock());
#else
		PushNetEvent(NETEVT_CONN_ERR, errno);
		close(GetSock());
#endif // WIN32
		return INVALID_SOCKET;
	}

	GetConnection()->SetID(GetSockId());

	SetState(SSTATE_ESTABLISH);
	PushNetEvent(NETEVT_ESTABLISH, 0);

#ifdef WIN32
	if (connect(GetSock(), (sockaddr*)(&m_stRemoteAddr), sizeof(m_stRemoteAddr)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "connect errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			return INVALID_SOCKET;
		}
	}

	if (false == PostRecv())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == PostRecv(), socket : %d, socket id : %d, errno : %d", GetSock(), GetSockId(), WSAGetLastError());

		return INVALID_SOCKET;
	}
#else
	if (connect(GetSock(), (sockaddr*)(&m_stRemoteAddr), sizeof(m_stRemoteAddr)) < 0)
	{
		if (errno != EINPROGRESS && errno != EINTR && errno != EAGAIN)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "connect errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			return INVALID_SOCKET;
		}
	}

	INT32 nError = 0;
	socklen_t nLen = sizeof(nError);
	if (getsockopt(GetSock(), SOL_SOCKET, SO_ERROR, &nError, &nLen) < 0)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "getsockopt errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

		return INVALID_SOCKET;
	}

	if (nError != 0)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "getsockopt errno : %d, socket : %d, socket id : %d", nError, GetSock(), GetSockId());

		return INVALID_SOCKET;
	}

	if (!m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

		return INVALID_SOCKET;
	}
#endif // WIN32

	return GetSock();
}

void FxUDPConnectSock::SetRemoteAddr(sockaddr_in& refstRemoteAddr)
{
	memcpy(&m_stRemoteAddr, &refstRemoteAddr, sizeof(refstRemoteAddr));
}

bool FxUDPConnectSock::PostClose()
{
#ifdef WIN32
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	// Post失败的时候再进入这个函数时可能会丢失一次//

	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), UINT32(0), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "PostQueuedCompletionStatus errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

		return false;
	}

	return true;
#else
	m_poIoThreadHandler->PushDelayCloseSock(this);
	return true;
#endif // WIN32
}

#ifdef WIN32
bool FxUDPConnectSock::PostRecv()
{
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

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

	unsigned char buffer_id = recv_window.free_buffer_id;
	unsigned char * buffer = recv_window.buffer[buffer_id];
	recv_window.free_buffer_id = buffer[0];

	// can't allocate buffer, disconnect.
	if (buffer_id >= recv_window.window_size)
	{
		return false;
	}

	m_stRecvIoData.stWsaBuf.buf = (char*)buffer;
	m_stRecvIoData.stWsaBuf.len = SlidingWindow::buffer_size;

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	int nSockAddr = sizeof(m_stRecvIoData.stRemoteAddr);
	if (SOCKET_ERROR == WSARecv(GetSock(), &m_stRecvIoData.stWsaBuf, 1, &dwReadLen, &dwFlags,
		&m_stRecvIoData.stOverlapped, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "WSARecvFrom errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			InterlockedCompareExchange(&m_nPostRecv, 0, 1);
			return false;
		}
	}

	return true;
}

bool FxUDPConnectSock::PostRecvFree()
{
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());
		return false;
	}
	LONG nPostRecv = InterlockedCompareExchange(&m_nPostRecv, 1, 0);
	if (0 != nPostRecv)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_nPostRecv == 1, socket : %d, socket id : %d", GetSock(), GetSockId());
		return false;
	}
	ZeroMemory(&m_stRecvIoData.stOverlapped, sizeof(m_stRecvIoData.stOverlapped));
	//post 失败的时候 再进入这个函数 可能会丢失一次
	if (!PostQueuedCompletionStatus(m_poIoThreadHandler->GetHandle(), UINT32(-1), (ULONG_PTR)this, &m_stRecvIoData.stOverlapped))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "PostQueuedCompletionStatus error : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());
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
				default:
				{
					Close();
				}
				break;
			}
		}
		break;
		default:
		{
			// 如果其他状态收到消息 肯定不对
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "state : %d, error", (UINT32)GetState());
			Close();        // 未知错误，不应该发生//
		}
		break;
	}
}
#else
void FxUDPConnectSock::OnParserIoEvent(int dwEvents)
{
	if (!IsConnected())
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
		if (errno == EINPROGRESS || errno == EINTR || errno == EAGAIN)
		{
			return;
		}
		PushNetEvent(NETEVT_ERROR, errno);
		Close();
	}
}

#endif // WIN32

bool FxUDPConnectSock::PostSend()
{
	{
		double time = GetTimeHandler()->GetMilliSecond();

		// check ack received time
		if (time - ack_recv_time > 5)
		{
			ack_recv_time = time;

			if (--ack_timeout_retry <= 0)
			{
				PostClose();
				return false;
			}
		}

		if (time < send_time)
			return true;

		bool force_retry = false;

		//if (status == ST_ESTABLISHED
		//	|| status == ST_FIN_WAIT_1)
		{
			// enter quick retry when get 3 same ack
			if (ack_same_count > 3)
			{
				if (quick_retry == false)
				{
					quick_retry = true;
					force_retry = true;

					send_window_threshhold = send_window_control / 2;
					if (send_window_threshhold < 2) send_window_threshhold = 2;
					send_window_control = send_window_threshhold + ack_same_count - 1;
					if (send_window_control > send_window.window_size)
						send_window_control = send_window.window_size;
				}
				else
				{
					// in quick retry
					// send_window_control increase 1 when get same ack 
					send_window_control += 1;
					if (send_window_control > send_window.window_size)
						send_window_control = send_window.window_size;
				}
			}
			else
			{
				// quick retry finished when get new ack
				if (quick_retry == true)
				{
					send_window_control = send_window_threshhold;
					quick_retry = false;
				}
			}

			// enter slow start when send data timeout 
			for (byte i = send_window.begin; i != send_window.end; i++)
			{
				byte id = i % send_window.window_size;
				unsigned short size = send_window.seq_size[id];

				if (send_window.seq_retry_count[id] > 0
					&& time >= send_window.seq_retry[id])
				{
					send_window_threshhold = send_window_control / 2;
					if (send_window_threshhold < 2) send_window_threshhold = 2;
					//send_window_control = 1;
					send_window_control = send_window_threshhold;
					//break;

					quick_retry = false;
					ack_same_count = 0;
					break;
				}
			}

			unsigned int offset = 0;
			char * send_buffer = NULL;
			unsigned int size = m_poSendBuf->GetOutCursorPtr(send_buffer);

			// put buffer to send window
			while ((send_window.free_buffer_id < send_window.window_size) &&	// there is a free buffer
				(size > 0))
			{
				// if send window more than send_window_control, break
				if (send_window.end - send_window.begin > send_window_control)
					break;

				byte id = send_window.end % send_window.window_size;

				// allocate buffer
				byte buffer_id = send_window.free_buffer_id;
				send_window.free_buffer_id = send_window.buffer[buffer_id][0];

				// send window buffer
				byte * buffer = send_window.buffer[buffer_id];

				// packet header
				UDPPacketHeader & packet = *(UDPPacketHeader*)buffer;
				packet.m_cStatus = GetState();
				packet.m_cSyn = send_window.end;
				packet.m_cAck = recv_window.begin - 1;

				// copy data
				unsigned int copy_offset = sizeof(packet);
				unsigned int copy_size = send_window.buffer_size - copy_offset;
				if (copy_size > size)
					copy_size = size;

				if (copy_size > 0)
				{
					memcpy(buffer + copy_offset, send_buffer + offset, copy_size);

					size -= copy_size;
					offset += copy_size;

					m_poSendBuf->DiscardBuff(copy_size);
				}

				// add to send window
				send_window.seq_buffer_id[id] = buffer_id;
				send_window.seq_size[id] = copy_size + copy_offset;
				send_window.seq_time[id] = time;
				send_window.seq_retry[id] = time;
				send_window.seq_retry_time[id] = retry_time;
				send_window.seq_retry_count[id] = 0;
				send_window.end++;
			}
		}

		// if there is no data to send, make an empty one
		if (send_window.begin == send_window.end)
		{
			if (time >= send_data_time)
			{
				if (send_window.free_buffer_id < send_window.window_size)
				{
					byte id = send_window.end % send_window.window_size;

					// allocate buffer
					byte buffer_id = send_window.free_buffer_id;
					send_window.free_buffer_id = send_window.buffer[buffer_id][0];

					// send window buffer
					byte * buffer = send_window.buffer[buffer_id];

					// packet header
					UDPPacketHeader & packet = *(UDPPacketHeader*)buffer;
					packet.m_cStatus = GetState();
					packet.m_cSyn = send_window.end;
					packet.m_cAck = recv_window.begin - 1;

					// add to send window
					send_window.seq_buffer_id[id] = buffer_id;
					send_window.seq_size[id] = sizeof(packet);
					send_window.seq_time[id] = time;
					send_window.seq_retry[id] = time;
					send_window.seq_retry_time[id] = retry_time;
					send_window.seq_retry_count[id] = 0;
					send_window.end++;
				}
			}
		}
		else
			send_data_time = time + send_data_frequency;

		// send packets
		for (byte i = send_window.begin; i != send_window.end; i++)
		{
			// if send packets more than send_window_control, break
			if (i - send_window.begin >= send_window_control)
				break;

			byte id = i % send_window.window_size;
			unsigned short size = send_window.seq_size[id];

			// send packet
			if (time >= send_window.seq_retry[id] || force_retry)
			{
				force_retry = false;

				char* buffer = (char*)send_window.buffer[send_window.seq_buffer_id[id]];

				// packet header
				UDPPacketHeader & packet = *(UDPPacketHeader*)buffer;
				packet.m_cStatus = GetState();
				packet.m_cSyn = i;
				packet.m_cAck = recv_window.begin - 1;

#ifdef WIN32
				ZeroMemory(&m_stSendIoData.stOverlapped, sizeof(m_stSendIoData.stOverlapped));

				m_stSendIoData.stWsaBuf.buf = buffer;
				m_stSendIoData.stWsaBuf.len = size;
				DWORD dwNumberOfBytesSent = 0;

				int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
					&m_stSendIoData.stOverlapped, NULL);
				if (0 != nRet)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						InterlockedCompareExchange(&m_nPostSend, 0, 1);

						UINT32 dwErr = WSAGetLastError();
						LogExe(LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

						return false;
					}
				}
#else
#endif
				// num send
				num_packets_send++;

				// num retry send
				if (time != send_window.seq_time[id])
					num_packets_retry++;

				send_time = time + send_frequency;
				send_data_time = time + send_data_frequency;
				send_ack = false;

				send_window.seq_retry_count[id]++;
				//send_window.seq_retry_time[id] *= 2;
				send_window.seq_retry_time[id] = 1.5 * retry_time;
				if (send_window.seq_retry_time[id] > 0.2) send_window.seq_retry_time[id] = 0.2;
				send_window.seq_retry[id] = time + send_window.seq_retry_time[id];
			}
		}

		// send ack
		if (send_ack)
		{
			UDPPacketHeader packet;
			packet.m_cStatus = GetState();
			packet.m_cSyn = send_window.begin - 1;
			packet.m_cAck = recv_window.begin - 1;

#ifdef WIN32
			ZeroMemory(&m_stSendIoData.stOverlapped, sizeof(m_stSendIoData.stOverlapped));

			m_stSendIoData.stWsaBuf.buf = (char*)(&packet);
			m_stSendIoData.stWsaBuf.len = sizeof(packet);
			DWORD dwNumberOfBytesSent = 0;

			int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
				&m_stSendIoData.stOverlapped, NULL);
			if (0 != nRet)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					InterlockedCompareExchange(&m_nPostSend, 0, 1);

					UINT32 dwErr = WSAGetLastError();
					LogExe(LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

					return false;
				}
			}

			return true;
#else
#endif
			send_time = time + send_frequency;
			send_ack = false;
		}
	}

	return true;
#ifdef WIN32
	if (false == IsConnected())
	{
		LogExe(LogLv_Error, "false == IsConnect(), socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//ThreadLog(LogLv_Critical, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poSendBuf->GetOutCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());
		// 不算失败，只是没有投递而已，下次可以继续//
		InterlockedCompareExchange(&m_nPostSend, 0, 1);
		return true;
	}

	nLen = 65536 < nLen ? 65536 : nLen;     // 最大64K

	if (m_poSendBuf->GetFreeLen() < 32 * 1024)
	{
		InterlockedCompareExchange(&m_nPostSend, 0, 1);
		return true;
	}

	m_stSendIoData.stWsaBuf.len = nLen;
	DWORD dwNumberOfBytesSent = 0;

	int nRet = WSASend(GetSock(), &m_stSendIoData.stWsaBuf, 1, &dwNumberOfBytesSent, 0,
		&m_stSendIoData.stOverlapped, NULL);
	if (0 != nRet)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			InterlockedCompareExchange(&m_nPostSend, 0, 1);

			UINT32 dwErr = WSAGetLastError();
			LogExe(LogLv_Error, "WSASend errno : %d, socket : %d, socket id : %d", WSAGetLastError(), GetSock(), GetSockId());

			return false;
		}
	}

	return true;
#else
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "NULL == m_poIoThreadHandler, socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poSendBuf->GetOutCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());
		// 不算失败，只是没有投递而已，下次可以继续//
		// modify: 在这里需要设置事件为EPOLLIN，否则OUT事件一直被设置，导致CPU很高//
		if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLIN, this))
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

			PushNetEvent(NETEVT_ERROR, errno);
			Close();
			return false;
		}
		return true;
	}

	m_bSending = true;
	nLen = UDP_VAL_SO_SNDLOWAT < nLen ? UDP_VAL_SO_SNDLOWAT : nLen;     // 最大64K

	int nRet = send(GetSock(), pSendBuf, nLen, 0);
	if (0 > nRet)
	{
		if (EAGAIN == errno)
		{
			if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

				m_bSending = false;
				return false;
			}
			return true;
		}

		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "send nRet < 0, socket : %d, socket id : %d", GetSock(), GetSockId());

		m_bSending = false;
		return false;
	}
	else if (0 == nRet)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "send 0 == nRet, socket : %d, socket id : %d", GetSock(), GetSockId());

		m_bSending = false;
		return false;
	}

	// 把成功发送了的从发送缓冲区丢弃//
	m_poSendBuf->DiscardBuff(nRet);

	if (false == m_poIoThreadHandler->ChangeEvent(GetSock(), EPOLLOUT | EPOLLIN, this))
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == m_poIoThreadHandler->ChangeEvent, socket : %d, socket id : %d", GetSock(), GetSockId());

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
	assert(0);
	return false;
#ifdef WIN32
	return PostSend();
#else
	if (false == IsConnected())
	{
		LogExe(LogLv_Error, "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return false;
	}

	if (NULL == m_poIoThreadHandler)
	{
		LogExe(LogLv_Error, "NULL == m_poIoThreadHandler, socket : %d, socket id : %d", GetSock(), GetSockId());

		Close();
		return false;
	}

	if (m_poSendBuf->GetFreeLen() < 32 * 1024)
	{
		return true;
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "session recv_size error need : %d, but : %d", dwLen, GetConnection()->GetRecvSize());
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

		memmove(GetConnection()->GetRecvBuf(), GetConnection()->GetRecvBuf() + GetDataHeader()->GetHeaderLength() + sizeof(UDPPacketHeader), dwLen - GetDataHeader()->GetHeaderLength() - sizeof(UDPPacketHeader));
		GetConnection()->OnRecv(dwLen - GetDataHeader()->GetHeaderLength() - sizeof(UDPPacketHeader));
	}
}

void FxUDPConnectSock::__ProcRecvPackageError(UINT32 dwLen)
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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "socket : %d, socket id : %d, connection id : %d, connection addr : %p",
				GetSock(), GetSockId(), GetConnection()->GetID(), GetConnection());
			return;
		}

		GetConnection()->OnSocketDestroy();
		GetConnection()->OnClose();

		SetConnection(NULL);
	}
	FxMySockMgr::Instance()->Release(this);
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
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == bRet errno : %d, socket : %d, socket id : %d", nErr, GetSock(), GetSockId());

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

	int nUsedLen = 0;
	int nParserLen = 0;
	int nLen = int(dwBytes);

	if (nLen < sizeof(UDPPacketHeader))
	{
		PostClose();
		return;
	}

	// packet received
	bool packet_received = false;

	{
		// allocate buffer
		byte buffer_id = recv_window.free_buffer_id;
		byte * buffer = recv_window.buffer[buffer_id];
		recv_window.free_buffer_id = buffer[0];

		// can't allocate buffer, disconnect.
		if (buffer_id >= recv_window.window_size)
		{
			PostClose();
			return;
		}

		// receive packet
		int n = nLen;

		// num bytes received
		num_bytes_received += n + 28;

		// packet header
		UDPPacketHeader & packet = *(UDPPacketHeader*)buffer;



			for (byte i = recv_window.begin; i != recv_window.end; i++)
			{
				byte id = i % recv_window.window_size;
				recv_window.seq_buffer_id[id] = recv_window.window_size;
				recv_window.seq_size[id] = 0;
				recv_window.seq_time[id] = 0;
				recv_window.seq_retry[id] = 0;
				recv_window.seq_retry_count[id] = 0;
			}


		{
			// receive ack, process send buffer.
			if (send_window.IsValidIndex(packet.m_cAck))
			{
				// got a valid packet
				ack_recv_time = GetTimeHandler()->GetMilliSecond();
				ack_timeout_retry = 3;

				// static value for calculate delay
				static const double err_factor = 0.125;
				static const double average_factor = 0.25;
				static const double retry_factor = 2;

				double rtt = delay_time;
				double err_time = 0;

				// send_window_control not more than double send_window_control 
				double send_window_control_max = send_window_control * 2;
				if (send_window_control_max > send_window.window_size)
					send_window_control_max = send_window.window_size;

				while (send_window.begin != (byte)(packet.m_cAck + 1))
				{
					byte id = send_window.begin % send_window.window_size;
					byte buffer_id = send_window.seq_buffer_id[id];

					// calculate delay only use no retry packet
					if (send_window.seq_retry_count[id] == 1)
					{
						// rtt(packet delay)
						rtt = GetTimeHandler()->GetMilliSecond() - send_window.seq_time[id];
						// err_time(difference between rtt and delay_time)
						err_time = rtt - delay_time;
						// revise delay_time with err_time 
						delay_time = delay_time + err_factor * err_time;
						// revise delay_average with err_time
						delay_average = delay_average + average_factor * (fabs(err_time) - delay_average);
					}

					// free buffer
					send_window.buffer[buffer_id][0] = send_window.free_buffer_id;
					send_window.free_buffer_id = buffer_id;
					send_window.begin++;

					// get new ack
					// if send_window_control more than send_window_threshhold in congestion avoidance,
					// else in slow start
					// in congestion avoidance send_window_control increase 1
					// in slow start send_window_control increase 1 when get send_window_control count ack
					if (send_window_control <= send_window_threshhold)
						send_window_control += 1;
					else
						send_window_control += 1 / send_window_control;

					if (send_window_control > send_window_control_max)
						send_window_control = send_window_control_max;
				}

				// calculate retry with delay_time and delay_average
				retry_time = delay_time + retry_factor * delay_average;
				if (retry_time < send_frequency) retry_time = send_frequency;
			}

			// get same ack
			if (ack_last == send_window.begin - 1)
				ack_same_count++;
			else
				ack_same_count = 0;

			// packet is valid
			if (recv_window.IsValidIndex(packet.m_cSyn))
			{
				byte id = packet.m_cSyn % recv_window.window_size;

				if (recv_window.seq_buffer_id[id] >= recv_window.window_size)
				{
					recv_window.seq_buffer_id[id] = buffer_id;
					recv_window.seq_size[id] = n;
					packet_received = true;

					//// no more buffer, try parse first.
					//if (recv_window.free_buffer_id >= recv_window.window_size)
					//	break;
					//else
					//	continue;
				}
			}
		}

		// free buffer.
		buffer[0] = recv_window.free_buffer_id;
		recv_window.free_buffer_id = buffer_id;





		if (send_window.begin == send_window.end)
			ack_same_count = 0;

		// record ack last
		ack_last = send_window.begin - 1;

		// update recv window
		if (packet_received)
		{
			byte last_ack = recv_window.begin - 1;
			byte new_ack = last_ack;
			bool parse_message = false;

			// calculate new ack
			for (byte i = recv_window.begin; i != recv_window.end; i++)
			{
				// recv buffer is invalid
				if (recv_window.seq_buffer_id[i % recv_window.window_size] >= recv_window.window_size)
					break;

				new_ack = i;
			}

			// ack changed
			if (new_ack != last_ack)
			{
				while (recv_window.begin != (byte)(new_ack + 1))
				{
					const byte head_size = sizeof(UDPPacketHeader);
					byte id = recv_window.begin % recv_window.window_size;
					byte buffer_id = recv_window.seq_buffer_id[id];
					byte * buffer = recv_window.buffer[buffer_id] + head_size;
					unsigned short size = recv_window.seq_size[id] - head_size;

					// copy buffer
					// add data to receive buffer
					char* pBuffRecv = NULL;
					int nLenRecv = m_poRecvBuf->GetInCursorPtr(pBuffRecv);
					if (size <= nLenRecv)
					{
						m_poRecvBuf->CostBuff(size);
						memcpy(pBuffRecv, buffer, size);
					}
					else
					{
						m_poRecvBuf->CostBuff(nLenRecv);
						memcpy(pBuffRecv, buffer, size);
						size -= nLenRecv;

						//不考虑包超长的情况 只处理包循环放了就可以了
						m_poRecvBuf->GetInCursorPtr(pBuffRecv);
						memcpy(pBuffRecv, buffer + nLenRecv, size);
						m_poRecvBuf->CostBuff(nLenRecv);
					}


						// free buffer
						recv_window.buffer[buffer_id][0] = recv_window.free_buffer_id;
						recv_window.free_buffer_id = buffer_id;

						// remove sequence
						recv_window.seq_size[id] = 0;
						recv_window.seq_buffer_id[id] = recv_window.window_size;
						recv_window.begin++;
						recv_window.end++;

						// mark for parse message
						parse_message = true;

						// send ack when get packet
						send_ack = true;
				}
			}

			// record receive syn last
			syn_last = recv_window.begin - 1;

			// parse message
			if (parse_message)
			{
				char *pUseBuf = NULL;
				nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
				if (nLen <= 0)
				{
					ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poRecvBuf->GetUsedCursorPtr <= 0, socket : %d, socket id : %d", GetSock(), GetSockId());

					InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
					m_dwLastError = NET_RECVBUFF_ERROR;
					PostClose();
					//m_oLock.UnLock();
					return;
				}

				char* pParseBuf = pUseBuf + nParserLen;
				UINT32 dwHeaderLen = GetDataHeader()->GetHeaderLength();
				GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen > nLen ? nLen : dwHeaderLen, 0);
				m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
				if (0 <= m_nPacketLen)
				{
					ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

					InterlockedCompareExchange(&m_nPostRecv, 0, m_nPostRecv);
					m_dwLastError = NET_RECVBUFF_ERROR;
					PostClose();
					//m_oLock.UnLock();
					return;
				}

				m_poRecvBuf->CostUsedBuff(m_nPacketLen);
				PushNetEvent(NETEVT_RECV, m_nPacketLen);


				InterlockedCompareExchange(&m_nPostRecv, m_nPostRecv - 1, m_nPostRecv);
				if (0 == m_nPostRecv)
				{
					if (false == PostRecv())
					{
						ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == PostRecv, socket : %d, socket id : %d", GetSock(), GetSockId());

						m_dwLastError = WSAGetLastError();
						PostClose();
					}
				}
			}
		}
	}
}

void FxUDPConnectSock::OnSend(bool bRet, int dwBytes)
{
	return;
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
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == PostSend, socket : %d, socket id : %d", GetSock(), GetSockId());

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
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == PostSend, socket : %d, socket id : %d", GetSock(), GetSockId());

			m_dwLastError = WSAGetLastError();
			PostClose();
		}
	}
}

#else
void FxUDPConnectSock::OnRecv()
{
	if (false == IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

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
		//ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poRecvBuf->GetInCursorPtr() = %d, socket : %d, socket id : %d", nLen, GetSock(), GetSockId());
		return;
	}

	nLen = UDP_VAL_SO_SNDLOWAT < nLen ? UDP_VAL_SO_SNDLOWAT : nLen;

	nLen = recv(GetSock(), pRecvBuff, nLen, 0);

	if (0 > nLen)
	{
		if ((errno != EAGAIN) && (errno != EINPROGRESS) && (errno != EINTR))
		{
			ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "recv errno : %d, socket : %d, socket id : %d", errno, GetSock(), GetSockId());

			PushNetEvent(NETEVT_ERROR, errno);
			Close();
			return;
		}
		return;
	}
	if (0 == nLen)
	{
		Close();
		return;
	}

	int nUsedLen = 0;
	int nParserLen = 0;
	m_poRecvBuf->CostBuff(nLen);

	char *pUseBuf = NULL;
	nLen = m_poRecvBuf->GetUsedCursorPtr(pUseBuf);
	if (nLen <= 0)
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "m_poRecvBuf->GetUsedCursorPtr() <= 0, socket : %d, socket id : %d", GetSock(), GetSockId());

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
			GetDataHeader()->BuildRecvPkgHeader(pParseBuf, (int)dwHeaderLen > nLen ? nLen : dwHeaderLen, 0);
			m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, nLen);
			if (-1 == m_nPacketLen)
			{
				ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

				nUsedLen += nLen;
				m_poRecvBuf->CostUsedBuff(nUsedLen);
				nUsedLen = 0;
				PushNetEvent(NETEVT_RECV_PACKAGE_ERROR, nLen);
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
							ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

							nUsedLen += nLen;
							m_poRecvBuf->CostUsedBuff(nUsedLen);
							nUsedLen = 0;
							PushNetEvent(NETEVT_RECV_PACKAGE_ERROR, nLen);
							nLen = 0;
						}
						GetDataHeader()->BuildRecvPkgHeader(pUseBuf, GetDataHeader()->GetHeaderLength() - nLen, nLen);
						pParseBuf = (char*)(GetDataHeader()->GetPkgHeader());
						m_nPacketLen = GetDataHeader()->ParsePacket(pParseBuf, GetDataHeader()->GetHeaderLength());
						if (0 >= m_nPacketLen)
						{
							ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "header error, socket : %d, socket id : %d", GetSock(), GetSockId());

							nUsedLen += nLen;
							m_poRecvBuf->CostUsedBuff(nUsedLen);
							nUsedLen = 0;
							PushNetEvent(NETEVT_RECV_PACKAGE_ERROR, nLen);
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
void FxUDPConnectSock::OnSend()
{
	if (!IsConnected())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == IsConnected(), socket : %d, socket id : %d", GetSock(), GetSockId());

		return;
	}

	m_bSending = false;
	if (!PostSend())
	{
		ThreadLog(LogLv_Error, m_poIoThreadHandler->GetFile(), m_poIoThreadHandler->GetLogFile(), "false == PostSend(), socket : %d, socket id : %d", GetSock(), GetSockId());

		PushNetEvent(NETEVT_ERROR, errno);
		Close();
		return;
	}
}


#endif // WIN32

