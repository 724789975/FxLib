#include "dbclient.h"
#include "dbreader.h"
#include "dbmodule.h"

FxRedisClient::FxRedisClient()
{
    __Reset();
	sprintf(m_szLogPath, "./%s_%p_log.txt", GetExeName(), this);
}

FxRedisClient::~FxRedisClient()
{
	if(m_poMySqlConn != NULL)
    {
		delete m_poMySqlConn;
        m_poMySqlConn = NULL;
    }
}

void FxRedisClient::__Reset()
{
    m_bTerminate		= false;
    m_poMySqlConn		= NULL;
    m_nLastReconnectTime= 0;
    m_bDbOK				= false;
    m_poThrdHandler		= NULL;
}

bool FxRedisClient::Start()
{
	FxCreateThreadHandler(this, true, m_poThrdHandler);
	if(NULL == m_poThrdHandler)
	{
		ThreadLog(LogLv_Error, m_pFile, m_szLogPath, "%s", "FxCreateThreadHandler failed");
        return false;
	}

	return true;
}

void FxRedisClient::Stop()
{
    m_bTerminate = true;
//     m_poThrdHandler->Stop();
	if(m_poThrdHandler != NULL)
	{
		m_poThrdHandler->WaitFor(FX_INFINITE);
		m_poThrdHandler->Release();
		m_poThrdHandler = NULL;
	}
}

bool FxRedisClient::ConnectRedis(const std::string& szHost, unsigned int dwPort)
{
	m_poMySqlConn = new FxRedisConnection;
	if(NULL == m_poMySqlConn)
	{
		LogExe(LogLv_Error, "%s", "new FxRedisConnection failed");
		return false;
	}

	if(!m_poMySqlConn->Connect(szHost, dwPort))                                                                              
	{
		LogExe(LogLv_Error, "Connect to redis %s:%d error", szHost.c_str(), dwPort);
		return false;
	}

	m_bDbOK = true;

	return true;
}

bool FxRedisClient::AddQuery(IRedisQuery *poDBCommand)
{
	m_oLock.Lock();
	m_oQuerys.push_back(poDBCommand);
	m_oLock.UnLock();

	return true;
}

INT32 FxRedisClient::Query(const char* pszSQL)
{
	if (false == m_bDbOK)
	{
		return FXDB_DISCONNECT;
	}

	return m_poMySqlConn->Query(pszSQL);
}

INT32 FxRedisClient::Query(const char* pszSQL, IRedisDataReader **ppReader)
{
    if (NULL != *ppReader)
    {
        return -1;
    }

    if (false == m_bDbOK)
    {
        *ppReader = NULL;
        return -1;
    }

    FxRedisReader* poReader = FxRedisModule::Instance()->FetchReader();
    if (NULL == poReader)
    {
        return -1;
    }
    *ppReader = poReader;

	INT32 nRetCode = m_poMySqlConn->Query( pszSQL, *poReader);

	if(-1 == nRetCode)
	{
		m_bDbOK = false;
	}

	return nRetCode;
}

void FxRedisClient::ThrdFunc()
{
	while(!m_bTerminate)
	{
		if(m_bDbOK)
		{
            if (!__DoQuery())
            {
                FxSleep(1);
            }
		}
		else
		{
			//每30秒重联一次
			time_t nNow = GetTimeHandler()->GetSecond();
			if(nNow - m_nLastReconnectTime > 10)
			{
				if(m_poMySqlConn->ReConnect())
				{
					m_bDbOK = true;
				}

				m_nLastReconnectTime = nNow;
			}
			else
			{
				__ClearQuery();
				FxSleep(1);
			}
		}
	}

	//退出前确保所有的请求都已执行完
	__ClearQuery();

	m_poMySqlConn->Close();
	m_bDbOK = false;
}

void FxRedisClient::__ClearQuery()
{
    while(__DoQuery()) {}
}

bool FxRedisClient::__DoQuery( void )
{
	IRedisQuery *poQuery = NULL;
    m_oLock.Lock();
    if(m_oQuerys.empty())
    {
        m_oLock.UnLock();
        return false;
    }

    poQuery = m_oQuerys.front();
    m_oQuerys.pop_front();

    m_oLock.UnLock();

    if(poQuery == NULL)
        return false;

    poQuery->OnQuery(this);
    FxRedisModule::Instance()->AddResult(poQuery);
    return true;
}

