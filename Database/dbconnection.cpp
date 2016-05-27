#include "dbconnection.h"
#include "strhelper.h"
#include <errmsg.h>

FxMySQLConnection::FxMySQLConnection(void)
{
	m_myConnection = NULL;
	m_bConnected   = false;
    memset((char*)&m_oAccount, 0, sizeof(SDBAccount));
}

FxMySQLConnection::~FxMySQLConnection(void)
{
	if (m_myConnection != NULL)
	{
		mysql_close(m_myConnection);
        m_myConnection = NULL;
	}
}

bool FxMySQLConnection::Connect(SDBAccount& account)
{
    memcpy((char*)&m_oAccount, (char*)&account, sizeof(SDBAccount));
    if ('\0' == m_oAccount.m_szCharactSet[0])
    {
        // 如果没有设置链接码，设置默认的连接码为 [latin1]
        StrSafeCopy(m_oAccount.m_szCharactSet, "latin1");
    }
	return Connect();
}

bool FxMySQLConnection::Connect()
{
	if (NULL == m_myConnection)
	{
		m_myConnection = mysql_init(NULL);
	}

	if (NULL == m_myConnection)
	{
		return false;
	}

	if('\0' != m_oAccount.m_szCharactSet[0])
	{
		if(mysql_options(m_myConnection, MYSQL_SET_CHARSET_NAME, m_oAccount.m_szCharactSet))
		{
			return false;
		}
	}

	MYSQL* myConn = mysql_real_connect(m_myConnection, m_oAccount.m_szHostName, m_oAccount.m_szLoginName, 
        m_oAccount.m_szLoginPwd, m_oAccount.m_szDBName, m_oAccount.m_wConnPort, NULL, CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS);
	if (NULL == myConn)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "connect error : %s", mysql_error(m_myConnection));
		Close();
		return false;
	}

	m_bConnected = true;

    char pszSql[1024] = {0};

#ifdef WIN32
	_snprintf(pszSql, 1024, "set names %s", m_oAccount.m_szCharactSet);
#else
	snprintf(pszSql, 1024, "set names %s", m_oAccount.m_szCharactSet);
#endif // WIN32

    

    if (Query(pszSql) < 0)
    {
        Close();
        return false;
    }

    if (false == __GetCharSetInfo())
    {
        Close();
        return false;
    }

    if (false == __IsValidCharSet())
    {
        Close();
        return false;
    }

	return true;
}

void FxMySQLConnection::Close()
{
	if(m_myConnection != NULL)
	{
		mysql_close(m_myConnection);
		m_myConnection = NULL;
	}

	m_bConnected = false;
}

bool FxMySQLConnection::ReConnect()
{
	Close();
	return Connect();
}

bool FxMySQLConnection::CheckConnect()
{
	if(NULL == m_myConnection)
	{
		return Connect();
	}

	if(mysql_ping(m_myConnection) != 0)
	{
		Close();
		return Connect();
	}

	return true;
}

INT32 FxMySQLConnection::Query(const char* pszSQL)
{
	if(false == CheckConnect())
	{
		return FXDB_ERR_CONN;
	}

	if (0 != mysql_query(m_myConnection, pszSQL))
	{
		UINT32 dwErr = mysql_errno(m_myConnection);
		if(CR_SERVER_LOST == dwErr || CR_CONN_HOST_ERROR == dwErr || CR_SERVER_GONE_ERROR == dwErr)
		{
			if(false == ReConnect())
			{
				return FXDB_ERR_CONN;
			}

			if(0 != mysql_query(m_myConnection, pszSQL))
			{
				return FXDB_ERR_UNKNOWN;
			}
		}
		else
		{
			return FXDB_ERR_UNKNOWN;
		}
	}

	mysql_free_result(mysql_store_result(m_myConnection));
    INT32 nRet = (INT32)mysql_affected_rows(m_myConnection);
    while(!mysql_next_result(m_myConnection))
    {
        MYSQL_RES* tmpRes = mysql_store_result(m_myConnection);
        if (NULL != tmpRes)
        {
            mysql_free_result(tmpRes);
            nRet = (INT32)mysql_affected_rows(m_myConnection);
        }
    }
	return nRet;
}

INT32 FxMySQLConnection::Query(const char* pszSQL, INT32 nLength, FxMySQLReader& reader)
{
    if(false == CheckConnect())
    {
        return FXDB_ERR_CONN;
    }

    if (0 != mysql_real_query(m_myConnection, pszSQL, nLength))
    {
        UINT32 dwErr = mysql_errno(m_myConnection);
        if(CR_SERVER_LOST == dwErr || CR_CONN_HOST_ERROR == dwErr || CR_SERVER_GONE_ERROR == dwErr)
        {
            if(false == ReConnect())
            {
                return FXDB_ERR_CONN;
            }

            if(0 != mysql_real_query(m_myConnection, pszSQL, nLength))
            {
                return FXDB_ERR_UNKNOWN;
            }
        }
        else
        {
            return FXDB_ERR_UNKNOWN;
        }
    }

    if (reader.m_res != NULL)
    {
        mysql_free_result(reader.m_res);
    }

    reader.m_res = mysql_store_result(m_myConnection);
    while(!mysql_next_result(m_myConnection))
    {
        MYSQL_RES* tmpRes = mysql_store_result(m_myConnection);
        if (NULL != tmpRes)
        {
            mysql_free_result(reader.m_res);
            reader.m_res = tmpRes;  // 取最后一个
        }
    }
    
    if(NULL == reader.m_res)
    {
        if(0 == mysql_field_count(m_myConnection))
        {
            return FXDB_NO_RESULT;
        }
        else
        {
            return FXDB_ERR_UNKNOWN;
        }

        return FXDB_HAS_RESULT;
    }
    else
    {
        if(mysql_num_rows(reader.m_res) != 0)
        {
            return FXDB_HAS_RESULT;
        }
        else
        {
            return FXDB_NO_RESULT;
        }
    }

    return  FXDB_ERR_UNKNOWN;
}

INT32 FxMySQLConnection::Query(const char* pszSQL, FxMySQLReader& reader)
{
	if(false == CheckConnect())
	{
		return FXDB_ERR_CONN;
	}

	if (0 != mysql_query(m_myConnection, pszSQL))
	{
		UINT32 dwErr = mysql_errno(m_myConnection);
		if(CR_SERVER_LOST == dwErr || CR_CONN_HOST_ERROR == dwErr || CR_SERVER_GONE_ERROR == dwErr)
		{
			if(false == ReConnect())
			{
				return FXDB_ERR_CONN;
			}

			if(0 != mysql_query(m_myConnection, pszSQL))
			{
				return FXDB_ERR_UNKNOWN;
			}
		}
		else
		{
			return FXDB_ERR_UNKNOWN;
		}
	}

	if (reader.m_res != NULL)
    {
		mysql_free_result(reader.m_res);
    }

	reader.m_res = mysql_store_result(m_myConnection);
    while(!mysql_next_result(m_myConnection))
    {
        MYSQL_RES* tmpRes = mysql_store_result(m_myConnection);
        if (NULL != tmpRes)
        {
            mysql_free_result(reader.m_res);
            reader.m_res = tmpRes;  // 取最后一个
        }
    }

	if(NULL == reader.m_res)
	{
		if(0 == mysql_field_count(m_myConnection))
		{
			return FXDB_NO_RESULT;
		}
		else
		{
			return FXDB_ERR_UNKNOWN;
		}

		return FXDB_HAS_RESULT;
	}
	else
	{
		if(mysql_num_rows(reader.m_res) != 0)
		{
			return FXDB_HAS_RESULT;
		}
		else
		{
			return FXDB_NO_RESULT;
		}
	}

	return  FXDB_ERR_UNKNOWN;
}

UINT32 FxMySQLConnection::EscapeString(const char* pszSrc, INT32 nSize, char* pszDest)
{
	if (NULL == m_myConnection)
	{
		return 0;
	}

#ifdef WIN32
	__try
#else
	try
#endif
	{
		return mysql_real_escape_string(m_myConnection, pszDest, pszSrc, nSize);
	}
#ifdef WIN32
	__except(EXCEPTION_EXECUTE_HANDLER)
#else
	catch(...)
#endif
	{
		return 0;
	}
}

UINT32 FxMySQLConnection::GetLastError() 
{ 
	if(m_myConnection!=NULL)
	{
		return (UINT32)mysql_errno(m_myConnection);
	}
	return  0;
}

const char* FxMySQLConnection::GetLastErrorString()
{ 
	if(m_myConnection!=NULL)
	{
		return mysql_error(m_myConnection);
	}
	return NULL;
}

bool FxMySQLConnection::__GetCharSetInfo()
{
    const char* pszSql = "select  @@character_set_client, \
                            @@character_set_connection,\
                            @@character_set_database,\
                            @@character_set_results,\
                            @@character_set_server,\
                            @@character_set_system";

    FxMySQLReader reader;

    if (FXDB_HAS_RESULT != Query(pszSql, reader))
    {
        return false;
    }

    reader.GetFirstRecord();
    if (true == reader.GetNextRecord())
    {
        for (int i = 0; i < CHARACTER_SET_COUNT; i++)
        {
            const char* pszCharSetName = reader.GetFieldValue(i);
            StrSafeCopy(m_szDBSCharSet[i], pszCharSetName);
        }
        return true;
    }
    return false;
}

#ifndef _WIN32
#define stricmp strcasecmp
#endif 
bool FxMySQLConnection::__IsValidCharSet()
{
    if (0 != stricmp(m_szDBSCharSet[CHARACTER_SET_CLIENT] , m_szDBSCharSet[CHARACTER_SET_CONNECTION]))
    {
        return false;
    }

    if (0 != stricmp(m_szDBSCharSet[CHARACTER_SET_CLIENT] , m_szDBSCharSet[CHARACTER_SET_DATABASE]))
    {
        return false;
    }

    if (0 != stricmp(m_szDBSCharSet[CHARACTER_SET_CLIENT] , m_szDBSCharSet[CHARACTER_SET_RESULTS]))
    {
        return false;
    }

    if (0 != stricmp(m_szDBSCharSet[CHARACTER_SET_CLIENT] , m_oAccount.m_szCharactSet))
    {
        return false;
    }
    return true;
}

IStmt* FxMySQLConnection::CreateStmt()
{
	if (!CheckConnect())
	{
		return NULL;
	}

	MYSQL_STMT* stmt = mysql_stmt_init(m_myConnection);
	if (!stmt)
		return NULL;
	FxMySQLStmt* pStmt = new FxMySQLStmt(this, stmt);
	return pStmt;
}
