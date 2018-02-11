#include "ChatSession.h"

ChatSession::ChatSession()
{
}


ChatSession::~ChatSession()
{
}

void ChatSession::Release(void)
{
	//LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
	CSocketSession::Release();
	CSessionFactory::Instance()->Release(this);
}

void ChatSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

