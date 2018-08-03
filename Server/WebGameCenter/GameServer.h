#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "ServerSession.h"

#include "GameServerBase.h"

class GameServer : public TSingleton<GameServer>, public CGameServerBase
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(std::string szPlayerListenIp, unsigned short wPlayerListenPort);
	bool Stop();

	BinaryServerSessionManager& GetBinaryServerSessionManager() { return m_oBinaryServerSessionManager; }

private:
	BinaryServerSessionManager m_oBinaryServerSessionManager;
	IFxListenSocket* m_pServerListenSocket;

	std::string m_szPlayerListenIp;
	unsigned short m_wPlayerListenPort;
};


#endif // !__GameServer_H__
