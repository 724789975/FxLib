#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "LoginSession.h"
#include "CenterSession.h"
#include "GameManagerSession.h"
#include "Team.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(unsigned int dwServerId, std::string szCenterIp, unsigned short wCenterPort, unsigned short wGameManagerPort);

	bool Stop();

	unsigned int GetServerId() { return m_dwServerId; }
	BinaryLoginSessionManager& GetLoginSessionManager() { return m_oBinaryLoginSessionManager; }
	BinaryGameManagerSessionManager& GetGameManagerSessionManager() { return m_oBinaryGameManagerSessionManager; }
	CTeamManager& GetTeamManager() { return m_oTeamManager; }

	unsigned short GetGameManagerPort() { return m_wGameManagerPort; }

private:

	BinaryLoginSessionManager m_oBinaryLoginSessionManager;

	CBinaryCenterSession m_oCenterSession;

	BinaryGameManagerSessionManager m_oBinaryGameManagerSessionManager;
	IFxListenSocket* m_pGameManagerListenSocket;

	CTeamManager m_oTeamManager;

	unsigned int m_dwServerId;

	std::string m_szCenterIp;
	unsigned short m_wCenterPort;

	unsigned short m_wGameManagerPort;
};


#endif // !__GameServer_H__
