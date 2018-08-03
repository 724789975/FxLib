#ifndef __GameServer_H__
#define __GameServer_H__

#include <vector>
#include <string>

#include "singleton.h"
#include "ServerSession.h"
#include "PlayerSession.h"
#include "LoginSession.h"
#include "CenterSession.h"
#include "TeamSession.h"

#include "GameServerBase.h"

class GameServer : public TSingleton<GameServer>, public CGameServerBase
{
public:
	GameServer();
	virtual ~GameServer();

	bool Init(unsigned int dwServerId, std::string szServerIp, std::string szCenterIp, unsigned short wCenterPort, unsigned short wServerListenPort, unsigned short wPlayerListenPort);
	bool Stop();

	//WebSocketPlayerSessionManager& GetWebSocketPlayerSessionManager() { return m_oWebSocketPlayerSessionManager; }
	//BinaryServerSessionManager& GetBinaryServerSessionManager() { return m_oBinaryServerSessionManager; }

	BinaryLoginSessionManager& GetLoginSessionManager() { return m_oBinaryLoginSessionManager; }
	BinaryTeamSessionManager& GetTeamSessionManager() { return m_oBinaryTeamSessionManager; }

	unsigned short GetServerListenPort() { return m_wServerListenPort; }

	std::string GetServerIp() { return m_szServerIp; }
	unsigned int GetServerId() { return m_dwServerId; }

private:
	WebSocketPlayerSessionManager m_oWebSocketPlayerSessionManager;
	BinaryServerSessionManager m_oBinaryServerSessionManager;

	BinaryLoginSessionManager m_oBinaryLoginSessionManager;

	CBinaryCenterSession m_oCenterSession;

	IFxListenSocket* m_pServerListenSocket;
	IFxListenSocket* m_pPlayerListenSocket;

	BinaryTeamSessionManager m_oBinaryTeamSessionManager;

	unsigned short m_wServerListenPort;
	unsigned short m_wPlayerListenPort;

	unsigned int m_dwServerId;

	std::string m_szServerIp;						//有些服务器外网ip需要显式的指定
	std::string m_szCenterIp;
	unsigned short m_wCenterPort;
};


#endif // !__GameServer_H__
