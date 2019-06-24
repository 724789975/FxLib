#include "GameServer.h"
#include "fxredis.h"


GameServer::GameServer()
	: m_pServerListenSocket(NULL)
	, m_pPlayerListenSocket(NULL)
	, m_pSlaveServerListenSocket(NULL)
	, m_wPlayerListenPort(0)
	, m_wServerListenPort(0)
	, m_wSlaveServerListenPort(0)
	, m_qwTeamId(0)
	, m_dwTeamServerId(0)
{
}

GameServer::~GameServer()
{
}

bool GameServer::Init(std::string szGameManagerIp, unsigned short wGameManagerPort, unsigned long long qwTeamId, unsigned int dwTeamServerId)
{
	m_qwTeamId = qwTeamId;
	m_dwTeamServerId = dwTeamServerId;
	IFxNet* pNet = FxNetGetModule();
	if (!pNet)
	{
		return false;
	}
	m_pPlayerListenSocket = pNet->Listen(&m_oWebSocketPlayerSessionManager, SLT_WebSocket, 0, m_wPlayerListenPort);
	if (m_pPlayerListenSocket == NULL)
	{
		return false;
	}
	m_pServerListenSocket = pNet->Listen(&m_oWebSocketServerSessionManager, SLT_WebSocket, 0, m_wServerListenPort);
	if (m_pServerListenSocket == NULL)
	{
		return false;
	}
	m_pSlaveServerListenSocket = pNet->Listen(&m_oWebSocketSlaveServerSessionManager, SLT_WebSocket, 0, m_wSlaveServerListenPort);
	if (m_pSlaveServerListenSocket == NULL)
	{
		return false;
	}

	if (pNet->TcpConnect(&m_oBinaryGameManagerSession, inet_addr(szGameManagerIp.c_str()), wGameManagerPort, true) == INVALID_SOCKET)
	{
		return false;
	}

	class RedisSetServerIp : public IRedisQuery
	{
	public:
		RedisSetServerIp(unsigned long long qwTeamId, std::string szGameIp) : m_qwTeamId(qwTeamId), m_szGameIp(szGameIp) {}
		~RedisSetServerIp() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "SET %s_%llu %s", RedisConstant::szGameIp, m_qwTeamId, m_szGameIp.c_str());
			poDBConnection->Query(szQuery);
		}
		virtual void OnResult(void) { }
		virtual void Release(void) { }

	private:
		unsigned long long m_qwTeamId;
		std::string m_szGameIp;
	};

	class RedisSetServerPort : public IRedisQuery
	{
	public:
		RedisSetServerPort(unsigned long long qwTeamId, unsigned int dwPort) : m_qwTeamId(qwTeamId), m_dwPort(dwPort) {}
		~RedisSetServerPort() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "SET %s_%llu %d", RedisConstant::szGamePort, m_qwTeamId, m_dwPort);
			poDBConnection->Query(szQuery);
		}
		virtual void OnResult(void) { }
		virtual void Release(void) { }

	private:
		unsigned long long m_qwTeamId;
		unsigned int m_dwPort;
	};

	RedisSetServerIp oRedisSetServerIp(qwTeamId, szGameManagerIp);
	RedisSetServerPort oRedisSetServerPort(qwTeamId, m_wPlayerListenPort);
	FxRedisGetModule()->QueryDirect(&oRedisSetServerIp);
	FxRedisGetModule()->QueryDirect(&oRedisSetServerPort);

	return true;
}

bool GameServer::Stop()
{
	// todo
	return false;
}

bool GameServer::GameEnd()
{
	LogExe(LogLv_Debug, "%s", "game end~~~~~");
	class RedisDelServerIp : public IRedisQuery
	{
	public:
		RedisDelServerIp(unsigned long long qwTeamId) : m_qwTeamId(qwTeamId) {}
		~RedisDelServerIp() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "DEL %s_%llu", RedisConstant::szGameIp, m_qwTeamId);
			poDBConnection->Query(szQuery);
		}
		virtual void OnResult(void) { }
		virtual void Release(void) { }

	private:
		unsigned long long m_qwTeamId;
	};

	class RedisDelServerPort : public IRedisQuery
	{
	public:
		RedisDelServerPort(unsigned long long qwTeamId) : m_qwTeamId(qwTeamId){}
		~RedisDelServerPort() {}

		virtual int					GetDBId(void) { return 0; }
		virtual void				OnQuery(IRedisConnection *poDBConnection)
		{
			char szQuery[64] = { 0 };
			sprintf(szQuery, "DEL %s_%llu", RedisConstant::szGamePort, m_qwTeamId);
			poDBConnection->Query(szQuery);
		}
		virtual void OnResult(void) { }
		virtual void Release(void) { }

	private:
		unsigned long long m_qwTeamId;
	};

	RedisDelServerIp oRedisDelServerIp(m_qwTeamId);
	RedisDelServerPort oRedisDelServerPort(m_qwTeamId);
	FxRedisGetModule()->QueryDirect(&oRedisDelServerIp);
	FxRedisGetModule()->QueryDirect(&oRedisDelServerPort);
	return false;
}
