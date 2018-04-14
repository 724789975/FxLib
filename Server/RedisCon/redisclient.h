#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <list>
#include <time.h>
#include <fxmeta.h>
#include "redisconnection.h"
#include "lock.h"

class FxRedisClient : public IRedisConnection, public IFxThread
{
public:
	FxRedisClient();
	virtual ~FxRedisClient();

	virtual INT32		Query(const char* pszSQL);
	virtual INT32		Query(const char* pszSQL, IRedisDataReader **ppReader);
	virtual void		ThrdFunc();
	virtual void		Stop();

	bool		        Start();
	bool				ConnectRedis(const std::string& szHost, unsigned int dwPort, std::string szPassword);
	bool		        AddQuery(IRedisQuery* poQuery);
	
private:
	void			    __ClearQuery();
	bool		        __DoQuery();
	void		        __Reset();

private:
	FxCriticalLock			m_oLock;
	IFxThreadHandler*	m_poThrdHandler;
	FxRedisConnection*	m_poMySqlConn;		
	std::list<IRedisQuery*>		 m_oQuerys;
	time_t				m_nLastReconnectTime;
	bool				m_bTerminate;
	bool				m_bDbOK;

	FILE*				m_pFile;
	char				m_szLogPath[64];
};

#endif	// __REDIS_CLIENT_H__

