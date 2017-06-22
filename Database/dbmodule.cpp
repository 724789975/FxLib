#include "dbmodule.h"

//IMPLEMENT_SINGLETON(FxDBModule)

FxDBModule::FxDBModule(void)
{
}

FxDBModule::~FxDBModule()
{
}

void FxDBModule::Release(void)
{
	Uninit();
	// !!!!曾经，下面这行代码放在DestroyInstance之下，导致了无声无息的崩溃，加此注释以示警戒
	FxDBModule::DestroyInstance();
	return;
}

const char * FxDBModule::GetModuleName(void)
{
	return FXDB_MODULENAME;
}

bool FxDBModule::Open(SDBConnInfo& DBInfo)
{
    DBInfo.m_stAccount.m_szCharactSet[MAX_DBCHARACTSET_SIZE - 1] = '\0';

	FxMySqlClient *poMySqlClient = FindDBClient(DBInfo.m_dwDBId);
	if(poMySqlClient != NULL)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "db id = %d already exist", DBInfo.m_dwDBId);
		return false;
	}

	poMySqlClient = new FxMySqlClient;
	if(NULL == poMySqlClient)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "FxDBModule::Open, new CMySqlClient error");
		return false;
	}

	if(false == poMySqlClient->ConnectDB(DBInfo.m_stAccount))
	{
		delete poMySqlClient;
		return false;
	}

	if(false == poMySqlClient->Start())
	{
		delete poMySqlClient;
		return false;
	}

	m_mapDBClient[DBInfo.m_dwDBId] = poMySqlClient;

	return true;
}

void FxDBModule::Close(UINT32 dwDBId)
{
	FxMySqlClient * poMySqlClient = FindDBClient(dwDBId);
	if(NULL == poMySqlClient)
    {
		return;
    }

	poMySqlClient->Stop();
	m_mapDBClient.erase(dwDBId);
	delete poMySqlClient;
}

bool FxDBModule::AddQuery(IQuery *poQuery)
{
	if(NULL == poQuery)
    {
		return false;
    }

	FxMySqlClient* poMySqlClient = FindDBClient(poQuery->GetDBId());
	if(NULL == poMySqlClient)
		return false;

	poMySqlClient->AddQuery(poQuery);
	return true;
}

bool FxDBModule::Run(UINT32 dwCount)
{
    bool bRet = false;
    for (UINT32 i = 0; i < dwCount; i++)
    {
        if (!__CallBackResult())
        {
            return bRet;
        }
        bRet = true;
    }

    return bRet;
}

bool FxDBModule::Init()
{
    return m_oReaderPool.Init(1000, 100);
}

void FxDBModule::Uninit()
{
    std::map<UINT32, FxMySqlClient*>::iterator it = m_mapDBClient.begin();
	for(; it != m_mapDBClient.end(); ++it)
	{
		FxMySqlClient * poMySqlClient = it->second;
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

FxMySqlClient * FxDBModule::FindDBClient(UINT32 dwDBId)
{
	std::map<UINT32, FxMySqlClient*>::iterator it = m_mapDBClient.find(dwDBId);
	if(it == m_mapDBClient.end())
    {
		return NULL;
    }

	return it->second;
}

void FxDBModule::AddResult(IQuery* poQuery)
{
    if (NULL == poQuery)
    {
        LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "FxDBModule::AddResult : NULL Query");
        return;
    }
    m_oLock.Lock();
    m_oResultList.push_back(poQuery);
    m_oLock.UnLock();
}

bool FxDBModule::__CallBackResult()
{
    IQuery *poQuery = NULL;

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

void FxDBModule::__ClearResult()
{
    while(__CallBackResult()) {}
}

FxMySQLReader* FxDBModule::FetchReader()
{
    return m_oReaderPool.FetchObj();
}

void FxDBModule::ReleaseReader(FxMySQLReader* poReader)
{
    m_oReaderPool.ReleaseObj(poReader);
}

