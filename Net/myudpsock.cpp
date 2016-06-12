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
		LogFun(LT_Screen | LT_File, LogLv_Error, "Set socket FIONBIO error : %d, socket : %d, socket id %d", WSAGetLastError(), GetSock(), GetSockId());
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

	return false;
}

bool FxUDPListenSock::Close()
{

	return false;
}

void FxUDPListenSock::Reset()
{

}

bool FxUDPListenSock::PushNetEvent(ENetEvtType eType, UINT32 dwValue)
{

	return false;
}

void FxUDPListenSock::ProcEvent()
{

}

#ifdef WIN32
void FxUDPListenSock::OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred)
{
	if (dwByteTransferred < sizeof(UDPPacketHeader))
	{
		return;
	}
}
#else
void FxUDPListenSock::OnParserIoEvent(int dwEvents)
{

}
#endif // WIN32

bool FxUDPListenSock::AddEvent()
{

	return false;
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

	DWORD dwReadLen = 0;
	DWORD dwFlags = 0;

	int dwSockAddr = sizeof(oSPerIoData.stRemoteAddr);

	if (WSARecvFrom(GetSock(), &oSPerIoData.stWsaBuf, 1, &dwReadLen, &dwFlags,
		(sockaddr*)(&oSPerIoData.stRemoteAddr), &dwSockAddr, &oSPerIoData.stOverlapped, NULL) == SOCKET_ERROR)
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

bool FxUDPListenSock::InitAcceptEx()
{
	// todo
	return false;
}

void FxUDPListenSock::OnAccept(SPerIoData* pstPerIoData)
{

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

void FxUDPConnectSock::OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred)
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

