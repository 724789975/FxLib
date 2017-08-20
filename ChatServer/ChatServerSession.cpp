#include "ChatServerSession.h"

ChatServerSession::ChatServerSession()
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::OnConnect(void)
{

}

void ChatServerSession::OnClose(void)
{

}

void ChatServerSession::OnError(UINT32 dwErrorNo)
{

}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

void ChatServerSession::Release(void)
{
	//CSocketSession::Release();
	//CSessionFactory::Instance()->Release(this);
}


//----------------------------------------------------------------------------
void ChatServerConnectedSession::OnClose(void)
{

}
//----------------------------------------------------------------------------
void ChatServerConnectSession::OnClose(void)
{

}
