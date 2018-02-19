#include "GameManagerSession.h"



CGameManagerSession::CGameManagerSession()
{
}


CGameManagerSession::~CGameManagerSession()
{
}

void CGameManagerSession::OnConnect(void)
{
}

void CGameManagerSession::OnClose(void)
{
}

void CGameManagerSession::OnError(UINT32 dwErrorNo)
{
}

void CGameManagerSession::OnRecv(const char * pBuf, UINT32 dwLen)
{
}

void CGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

void CGameManagerSession::OnGameNotifyGameManagerInfo(const char * pBuf, UINT32 dwLen)
{
}

//////////////////////////////////////////////////////////////////////////
CBinaryGameManagerSession::CBinaryGameManagerSession()
{
}

CBinaryGameManagerSession::~CBinaryGameManagerSession()
{
}

void CBinaryGameManagerSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}
