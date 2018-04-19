#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "LoginSession.h"
#include "CenterSession.h"
#include "Team.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(unsigned int dwServerId, std::string szCenterIp, unsigned short wCenterPort);
	bool Stop();

	unsigned int GetServerId() { return m_dwServerId; }
	BinaryLoginSessionManager& GetLoginSessionManager() { return m_oBinaryLoginSessionManager; }
	CTeamManager& GetTeamManager() { return m_oTeamManager; }

private:

	BinaryLoginSessionManager m_oBinaryLoginSessionManager;

	CBinaryCenterSession m_oCenterSession;

	IFxListenSocket* m_pServerListenSocket;
	IFxListenSocket* m_pPlayerListenSocket;

	CTeamManager m_oTeamManager;

	unsigned int m_dwServerId;

	std::string m_szCenterIp;
	unsigned short m_wCenterPort;
};


#endif // !__GameServer_H__
