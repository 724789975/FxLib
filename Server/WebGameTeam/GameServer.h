#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "LoginSession.h"
#include "CenterSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(unsigned int dwServerId, std::string szCenterIp, unsigned short wCenterPort);
	bool Stop();

	BinaryLoginSessionManager& GetLoginSessionManager() { return m_oBinaryLoginSessionManager; }

	unsigned int GetServerid() { return m_dwServerId; }

private:

	BinaryLoginSessionManager m_oBinaryLoginSessionManager;

	CBinaryCenterSession m_oCenterSession;

	IFxListenSocket* m_pServerListenSocket;
	IFxListenSocket* m_pPlayerListenSocket;


	unsigned int m_dwServerId;

	std::string m_szCenterIp;
	unsigned short m_wCenterPort;
};


#endif // !__GameServer_H__
