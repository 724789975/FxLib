#include "redisquery.h"
#include "redisreader.h"
#include "redismodule.h"

FxRedisQuery::FxRedisQuery()
{
}

FxRedisQuery::~FxRedisQuery()
{
}

bool FxRedisQuery::ConnectRedis(const std::string& szHost, unsigned int dwPort, std::string szPassword)
{
	if(!m_oMySqlConn.Connect(szHost, dwPort, szPassword))                                                                              
	{
		LogExe(LogLv_Error, "Connect to redis %s:%d error", szHost.c_str(), dwPort);
		return false;
	}

	return true;
}

void FxRedisQuery::Query(IRedisQuery *poQuery)
{
	poQuery->OnQuery(this);
	poQuery->OnResult();
	poQuery->Release();
}

INT32 FxRedisQuery::Query(const char* pszCMD)
{
	return m_oMySqlConn.Query(pszCMD);
}

INT32 FxRedisQuery::Query(const char* pszCMD, IRedisDataReader **ppReader)
{
    if (NULL != *ppReader)
    {
        return -1;
    }

    FxRedisReader* poReader = FxRedisModule::Instance()->FetchReader();
    if (NULL == poReader)
    {
        return -1;
    }
    *ppReader = poReader;

	INT32 nRetCode = m_oMySqlConn.Query( pszCMD, *poReader);

	return nRetCode;
}


