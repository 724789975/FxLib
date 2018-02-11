#include "SlaveServerSession.h"
#include "netstream.h"
#include "gamedefine.h"

const static unsigned int g_dwSlaveServerSessionBuffLen = 64 * 1024;
static char g_pSlaveServerSessionBuf[g_dwSlaveServerSessionBuffLen];

CSlaveServerSession::CSlaveServerSession()
{
}


CSlaveServerSession::~CSlaveServerSession()
{
}

void CSlaveServerSession::OnConnect(void)
{
}

void CSlaveServerSession::OnClose(void)
{
}

void CSlaveServerSession::OnError(UINT32 dwErrorNo)
{
}

void CSlaveServerSession::OnRecv(const char * pBuf, UINT32 dwLen)
{
}

void CSlaveServerSession::Release(void)
{
}
