#ifndef __REDIS_QUERY_H__
#define __REDIS_QUERY_H__

#include <list>
#include <time.h>
#include <fxmeta.h>
#include "redisconnection.h"
#include "lock.h"

class FxRedisQuery : public IRedisConnection
{
public:
	FxRedisQuery();
	virtual ~FxRedisQuery();

	virtual int		Query(const char* pszSQL);
	virtual int		Query(const char* pszSQL, IRedisDataReader **ppReader);

	bool ConnectRedis(const std::string& szHost, unsigned int dwPort, std::string szPassword);
	void		        Query(IRedisQuery* poQuery);
	
private:
	FxRedisConnection	m_oMySqlConn;		
};

#endif	// __REDIS_QUERY_H__

