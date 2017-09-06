#include "ChatServerManager.h"



ChatServerManager::ChatServerManager()
{
}


ChatServerManager::~ChatServerManager()
{
}

bool ChatServerManager::Init()
{
	if (m_oChatSessionManager.Init() == false)
	{
		return false;
	}
	return true;
}
