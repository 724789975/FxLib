#include "ChatServerSession.h"

ChatServerSession::ChatServerSession()
{
}


ChatServerSession::~ChatServerSession()
{
}

void ChatServerSession::Release(void)
{
	CSocketSession::Release();
	CSessionFactory::Instance()->Release(this);
}

void ChatServerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

