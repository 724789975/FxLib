#ifndef __DBCONNECTION_H_2009_0824__
#define __DBCONNECTION_H_2009_0824__

#include "dbreader.h"
#include "dbstmt.h"

enum
{
    CHARACTER_SET_CLIENT = 0,
    CHARACTER_SET_CONNECTION,
    CHARACTER_SET_DATABASE,
    CHARACTER_SET_RESULTS,
    CHARACTER_SET_SERVER,
    CHARACTER_SET_SYSTEM,
    CHARACTER_SET_COUNT,
};

class FxMySQLConnection
{
public:
	FxMySQLConnection(void);
	virtual ~FxMySQLConnection(void);

    bool            Connect(SDBAccount& account);
    bool            Connect();
	bool            ReConnect();
    bool            CheckConnect();
	void            Close();

	UINT32          GetAffectedRows() { return (UINT32)mysql_affected_rows(m_myConnection); }
	UINT32          EscapeString(const char* pszSrc, INT32 nSize, char* pszDest);
	INT32           Query(const char* pszSQL, FxMySQLReader& rcdSet);
	INT32           Query(const char* pszSQL, INT32 nLength, FxMySQLReader& rcdSet);
	INT32           Query(const char* pszSQL);
	UINT32          GetLastError();
	const char*     GetLastErrorString();
	IStmt*			CreateStmt();

private:
    bool            __GetCharSetInfo();               
    bool            __IsValidCharSet();   

private:
	MYSQL*          m_myConnection;
	bool            m_bConnected;
    SDBAccount      m_oAccount;
    char            m_szDBSCharSet[CHARACTER_SET_COUNT][MAX_DBCHARACTSET_SIZE];
};

#endif  // __DBCONNECTION_H_2009_0824__

