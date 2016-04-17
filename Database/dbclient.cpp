#include "dbclient.h"
#include "dbreader.h"
#include "dbmodule.h"

FxMySqlClient::FxMySqlClient()
{
    __Reset();
}

FxMySqlClient::~FxMySqlClient()
{
	if(m_poMySqlConn != NULL)
    {
		delete m_poMySqlConn;
        m_poMySqlConn = NULL;
    }
}

void FxMySqlClient::__Reset()
{
    m_bTerminate		= false;
    m_poMySqlConn		= NULL;
    m_nLastReconnectTime= 0;
    m_bDbOK				= false;
    m_poThrdHandler		= NULL;
}

bool FxMySqlClient::Start()
{
	m_poThrdHandler = FxCreateThreadHandler(this, true);
	if(NULL == m_poThrdHandler)
	{
		LogScreen("FxCreateThreadHandler failed");
        return false;
	}

	return true;
}

void FxMySqlClient::Stop()
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

bool FxMySqlClient::ConnectDB(SDBAccount& account)
{
	m_poMySqlConn = new FxMySQLConnection;
	if(NULL == m_poMySqlConn)
	{
		LogScreen("CMySqlClient::ConnectDB, new CMySQLConnection failed");
		return false;
	}

	if(!m_poMySqlConn->Connect(account))                                                                              
	{
		LogScreen("Connect to Database %s error: %d, %s", account.m_szDBName, m_poMySqlConn->GetLastError(), m_poMySqlConn->GetLastErrorString());
		return false;
	}

	m_bDbOK = true;

	return true;
}

bool FxMySqlClient::AddQuery(IQuery *poDBCommand)
{
	m_oLock.Lock();
	m_oQuerys.push_back(poDBCommand);
	m_oLock.UnLock();

	return true;
}

UINT32 FxMySqlClient::EscapeString(const char* pszSrc, INT32 nSrcLen, char* pszDest, INT32 nDestLen)
{
	if(NULL == m_poMySqlConn)
	{
		return 0;
	}

	return m_poMySqlConn->EscapeString(pszSrc, nSrcLen, pszDest);
}

INT32 FxMySqlClient::Query(const char* pszSQL)
{
	if (false == m_bDbOK)
	{
		return FXDB_DISCONNECT;
	}

	return m_poMySqlConn->Query(pszSQL);
}

INT32 FxMySqlClient::Query(const char* pszSQL, IDataReader **ppReader)
{
    if (NULL != *ppReader)
    {
        return FXDB_ERR_READER;
    }

    if (false == m_bDbOK)
    {
        *ppReader = NULL;
        return FXDB_DISCONNECT;
    }

    FxMySQLReader* poReader = FxDBModule::Instance()->FetchReader();
    if (NULL == poReader)
    {
        return FXDB_ERR_READER;
    }
    *ppReader = poReader;

	INT32 nRetCode = m_poMySqlConn->Query( pszSQL, (INT32)strlen(pszSQL), *poReader);

	if(FXDB_ERR_CONN == nRetCode)
	{
		m_bDbOK = false;
	}

	return nRetCode;
}

UINT32 FxMySqlClient::GetLastError(void)
{
	return m_poMySqlConn->GetLastError();
}

const char * FxMySqlClient::GetLastErrorString(void)
{
	return m_poMySqlConn->GetLastErrorString();
}

void FxMySqlClient::ThrdFunc()
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
            time_t nNow = time(NULL);
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

void FxMySqlClient::__ClearQuery()
{
    while(__DoQuery()) {}
}

bool FxMySqlClient::__DoQuery( void )
{
    IQuery *poQuery = NULL;
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
    FxDBModule::Instance()->AddResult(poQuery);
    return true;
}

IStmt* FxMySqlClient::CreateStmt()
{
	return m_poMySqlConn->CreateStmt();
}

