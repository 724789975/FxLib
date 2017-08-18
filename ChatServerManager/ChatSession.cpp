#include "ChatSession.h"

ChatSession::ChatSession()
{
}


ChatSession::~ChatSession()
{
}

void ChatSession::Release(void)
{
	CSocketSession::Release();
	CSessionFactory::Instance()->Release(this);
}

void ChatSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

