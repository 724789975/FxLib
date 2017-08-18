#include "GameSession.h"

GameSession::GameSession()
{
}


GameSession::~GameSession()
{
}

void GameSession::Release(void)
{
	CSocketSession::Release();
	CSessionFactory::Instance()->Release(this);
}

void GameSession::OnRecv(const char* pBuf, UINT32 dwLen)
{

}

