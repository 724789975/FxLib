#include "myudpsock.h"

FxUDPListenSock::FxUDPListenSock()
{

}

FxUDPListenSock::~FxUDPListenSock()
{

}

bool FxUDPListenSock::Init()
{
	return false;
}

void FxUDPListenSock::OnRead()
{

}

void FxUDPListenSock::OnWrite()
{

}

bool FxUDPListenSock::Listen(UINT32 dwIP, UINT16 wPort)
{

	return false;
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
bool FxUDPListenSock::PostAccept(SPerIoData& oSPerIoData)
{
	// todo
	return false;
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

