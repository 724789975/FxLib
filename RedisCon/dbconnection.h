#ifndef __REDISCONNECTION_H_20017_1210__
#define __REDISCONNECTION_H_20017_1210__

#include "hiredis.h"
#include <set>
#include <vector>
#include <map>
#include "lock.h"
#include "dbreader.h"

class FxRedisConnection
{
public:
	FxRedisConnection(void);
	virtual ~FxRedisConnection(void);

    bool			Connect(const std::string& szHost, unsigned int dwPort);
	bool            ReConnect();
	bool			Close();

	INT32           Query(const char* pszCMD, FxRedisReader& rcdSet);
	INT32           Query(const char* pszCMD);

private:
	bool			CheckConnection();

private:
	redisContext* m_pRedisContext;
	std::string m_szHost;
	unsigned int m_dwPort;
};

#endif  // __REDISCONNECTION_H_20017_1210__

