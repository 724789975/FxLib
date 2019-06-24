#include "redismodule.h"

//IMPLEMENT_SINGLETON(FxDBModule)

FxRedisModule::FxRedisModule(void)
{
}

FxRedisModule::~FxRedisModule()
{
}

void FxRedisModule::Release(void)
{
	Uninit();
	// !!!!曾经，下面这行代码放在DestroyInstance之下，导致了无声无息的崩溃，加此注释以示警戒
	FxRedisModule::DestroyInstance();
	return;
}

const char * FxRedisModule::GetModuleName(void)
{
	return "FXREDIS";
}

bool FxRedisModule::Open(std::string szHost, unsigned int dwPort, std::string szPassword, unsigned int dwRedisId)
{
	FxRedisClient *poMySqlClient = FindDBClient(dwRedisId);
	if(poMySqlClient != NULL)
	{
		LogExe(LogLv_Error, "db id = %d already exist", dwRedisId);
		return false;
	}

	poMySqlClient = new FxRedisClient;
	if(NULL == poMySqlClient)
	{
		LogExe(LogLv_Error, "%s", "FxDBModule::Open, new CMySqlClient error");
		return false;
	}

	if(false == poMySqlClient->ConnectRedis(szHost, dwPort, szPassword))
	{
		delete poMySqlClient;
		return false;
	}

	if(false == poMySqlClient->Start())
	{
		delete poMySqlClient;
		return false;
	}

	m_mapDBClient[dwRedisId] = poMySqlClient;

	m_mapRedisQuery[dwRedisId].ConnectRedis(szHost, dwPort, szPassword);

	return true;
}

void FxRedisModule::Close(unsigned int dwDBId)
{
	FxRedisClient * poMySqlClient = FindDBClient(dwDBId);
	if(NULL == poMySqlClient)
    {
		return;
    }

	poMySqlClient->Stop();
	m_mapDBClient.erase(dwDBId);
	delete poMySqlClient;
}

bool FxRedisModule::AddQuery(IRedisQuery*poQuery)
{
	if(NULL == poQuery)
    {
		return false;
    }

	FxRedisClient* poMySqlClient = FindDBClient(poQuery->GetDBId());
	if(NULL == poMySqlClient)
		return false;

	poMySqlClient->AddQuery(poQuery);
	return true;
}

bool FxRedisModule::Run(unsigned int dwCount)
{
    bool bRet = false;
    for (unsigned int i = 0; i < dwCount; i++)
    {
        if (!__CallBackResult())
        {
            return bRet;
        }
        bRet = true;
    }

    return bRet;
}

void FxRedisModule::QueryDirect(IRedisQuery* poQuery)
{
	m_mapRedisQuery[poQuery->GetDBId()].Query(poQuery);
}

bool FxRedisModule::Init()
{
    return m_oReaderPool.Init(1000, 1000);
}

void FxRedisModule::Uninit()
{
    std::map<unsigned int, FxRedisClient*>::iterator it = m_mapDBClient.begin();
	for(; it != m_mapDBClient.end(); ++it)
	{
		FxRedisClient * poMySqlClient = it->second;
        if (NULL == poMySqlClient)
        {
            continue;
        }

		poMySqlClient->Stop();
		delete poMySqlClient;
	}

	m_mapDBClient.clear();

	__ClearResult();
}

FxRedisClient * FxRedisModule::FindDBClient(unsigned int dwDBId)
{
	std::map<unsigned int, FxRedisClient*>::iterator it = m_mapDBClient.find(dwDBId);
	if(it == m_mapDBClient.end())
    {
		return NULL;
    }

	return it->second;
}

void FxRedisModule::AddResult(IRedisQuery* poQuery)
{
    if (NULL == poQuery)
    {
        LogExe(LogLv_Error, "%s", "FxDBModule::AddResult : NULL Query");
        return;
    }
    m_oLock.Lock();
    m_oResultList.push_back(poQuery);
    m_oLock.UnLock();
}

bool FxRedisModule::__CallBackResult()
{
    IRedisQuery *poQuery = NULL;

    m_oLock.Lock();
    if (m_oResultList.empty())
    {
        m_oLock.UnLock();
        return false;
    }

    poQuery = m_oResultList.front();
    m_oResultList.pop_front();
    m_oLock.UnLock();

    if(NULL == poQuery)
    {
        return false;
    }

    poQuery->OnResult();
    poQuery->Release();
    return true;
}

void FxRedisModule::__ClearResult()
{
    while(__CallBackResult()) {}
}

FxRedisReader* FxRedisModule::FetchReader()
{
    return m_oReaderPool.FetchObj();
}

void FxRedisModule::ReleaseReader(FxRedisReader* poReader)
{
    m_oReaderPool.ReleaseObj(poReader);
}

