#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "ServerSession.h"
#include "PlayerSession.h"
#include "PlayerManager.h"
#include "CenterSession.h"
#include "LoginSession.h"
#include "GameManagerSession.h"

class GameServer : public TSingleton<GameServer>
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(unsigned int dwServerId, std::string szPlayerListenIp, unsigned short wPlayerListenPort, std::string szGameCenterIp, unsigned short wGameCenterPort, unsigned short wLoginPort, unsigned short wTeamPort, unsigned short wGameManagerPort);
	bool Stop();

	WebSocketPlayerSessionManager& GetPlayerSessionManager() { return m_oWebSocketPlayerSessionManager; }

	PlayerManager& GetPlayerManager() { return m_oPlayerManager; }

	BinaryLoginSessionManager& GetLoginSessionManager() { return m_oLoginSessionManager; }
	BinaryGameManagerSessionManager& GetGameManagerSessionManager() { return m_oGameManagerSessionManager; }

	unsigned int GetServerid() { return m_dwServerId; }
	unsigned short GetLoginPort() { return m_wLoginPort; }
	unsigned short GetTeamPort() { return m_wTeamPort; }
	unsigned short GetGameManagerPort() { return m_wGameManagerPort; }

private:
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	IFxListenSocket* m_pPlayerListenSocket;

	CBinaryCenterSession m_oCenterSession;

	PlayerManager m_oPlayerManager;

	BinaryLoginSessionManager m_oLoginSessionManager;
	IFxListenSocket* m_pLoginListenSocket;

	BinaryGameManagerSessionManager m_oGameManagerSessionManager;
	IFxListenSocket* m_pGameManagerListenSocket;

	unsigned int m_dwServerId;

	std::string m_szPlayerListenIp;
	unsigned short m_wPlayerListenPort;

	std::string m_szGameCenterIp;
	unsigned short m_wGameCenterPort;

	unsigned short m_wLoginPort;
	unsigned short m_wTeamPort;
	unsigned short m_wGameManagerPort;
};


#endif // !__GameServer_H__
