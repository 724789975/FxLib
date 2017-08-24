#include "ChatManagerSession.h"
#include "chatdefine.h"
#include "netstream.h"


ChatManagerSession::ChatManagerSession()
{
}


ChatManagerSession::~ChatManagerSession()
{
}

void ChatManagerSession::OnConnect(void)
{

}

void ChatManagerSession::OnClose(void)
{

}

void ChatManagerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatManagerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	Protocol::EChatProtocol eProrocol = (Protocol::EChatProtocol)(*((UINT32*)pBuf));
	const char* pData = pBuf + sizeof(UINT32);
	dwLen -= sizeof(UINT32);

	switch (eProrocol)
	{
		case Protocol::CHAT_MANAGER_NOTIFY_CHAT_INFO:	OnNotifyChatInfo(pData, dwLen);	break;
		default:	Assert(0);	break;
	}
}

void ChatManagerSession::Release(void)
{

}

void ChatManagerSession::OnNotifyChatInfo(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
}
