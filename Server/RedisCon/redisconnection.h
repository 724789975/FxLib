#ifndef __REDISCONNECTION_H_20017_1210__
#define __REDISCONNECTION_H_20017_1210__

#include "hiredis.h"
#include <set>
#include <vector>
#include <map>
#include "lock.h"
#include "redisreader.h"

class FxRedisConnection
{
public:
	FxRedisConnection(void);
	virtual ~FxRedisConnection(void);

    bool			Connect(const std::string& szHost, unsigned int dwPort, std::string szPassword);
	bool            ReConnect();
	bool			Close();

	int           Query(const char* pszCMD, FxRedisReader& rcdSet);
	int           Query(const char* pszCMD);

private:
	bool			CheckConnection();

private:
	redisContext* m_pRedisContext;
	std::string m_szHost;
	std::string m_szPassword;
	unsigned int m_dwPort;
};

#endif  // __REDISCONNECTION_H_20017_1210__

