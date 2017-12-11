#include "dbstmt.h"

#include "dbconnection.h"


FxMySQLStmt::FxMySQLStmt( FxRedisConnection* pConnection, MYSQL_STMT* pStmt )
	:m_pDBConnection(pConnection), m_pStmt(pStmt), 
	m_pParamsBind(NULL), m_pResultsBind(NULL), 
	m_dwCurBindParamPos(0), m_dwParamsNum(0), 
	m_dwCurBindResultPos(0), m_dwResultsNum(0)
{

}

FxMySQLStmt::~FxMySQLStmt()
{
	delete [] m_pParamsBind;
	delete [] m_pResultsBind;
}

bool FxMySQLStmt::Prepare( const char* pszSQL, UINT32 dwParamNum, UINT32 dwResultNum )
{
	if (mysql_stmt_prepare(m_pStmt, pszSQL, strlen(pszSQL)) != 0)
		return false;

	m_dwParamsNum = dwParamNum;
	m_dwResultsNum = dwResultNum;

	m_pParamsBind = new MYSQL_BIND[dwParamNum];
	memset(m_pParamsBind, 0, dwParamNum * sizeof(MYSQL_BIND));
	m_pResultsBind = new MYSQL_BIND[dwResultNum];
	memset(m_pResultsBind, 0, dwResultNum * sizeof(MYSQL_BIND));

	return true;
}

void FxMySQLStmt::BindParam(enum_field_types eFieldType, void* pBuf, UINT32 dwSize)
{
	unsigned long dwLength = dwSize;
	m_pParamsBind[m_dwCurBindParamPos].buffer_type = eFieldType;
	m_pParamsBind[m_dwCurBindParamPos].buffer = pBuf;
	m_pParamsBind[m_dwCurBindParamPos].buffer_length = dwSize;
	m_pParamsBind[m_dwCurBindParamPos].length = &dwLength;
	m_pParamsBind[m_dwCurBindParamPos].is_null = 0;

	++ m_dwCurBindParamPos;
}

bool FxMySQLStmt::BindParamComplete()
{
	if (mysql_stmt_bind_param(m_pStmt, m_pParamsBind) < 0)
		return false;
	return true;
}

void FxMySQLStmt::BindResult(enum_field_types eFieldType, void* pBuf, UINT32 dwBufSize, UINT32* pdwLength)
{
	m_pResultsBind[m_dwCurBindResultPos].buffer_type = eFieldType;
	m_pResultsBind[m_dwCurBindResultPos].buffer = pBuf;
	m_pResultsBind[m_dwCurBindResultPos].buffer_length = dwBufSize;
	m_pResultsBind[m_dwCurBindResultPos].length = (unsigned long*)pdwLength;

	++ m_dwCurBindResultPos;
}

bool FxMySQLStmt::BindResultComplete()
{
	if (mysql_stmt_bind_result(m_pStmt, m_pResultsBind) < 0)
		return false;
	return true;
}

bool FxMySQLStmt::Execute()
{
	if (mysql_stmt_execute(m_pStmt) != 0)
		return false;

	return true;
}

const char* FxMySQLStmt::GetErrorString()
{
	return mysql_stmt_error(m_pStmt);
}

UINT32 FxMySQLStmt::AffectedRows()
{
	return (UINT32)(mysql_stmt_affected_rows(m_pStmt));
}

bool FxMySQLStmt::Restore()
{
	if (mysql_stmt_store_result(m_pStmt) != 0)
		return false;
	return true;
}

bool FxMySQLStmt::Fetch()
{
	if (mysql_stmt_fetch(m_pStmt) != 0)
		return false;
	return true;
}

UINT32 FxMySQLStmt::NumOfRows()
{
	return (UINT32)(mysql_stmt_num_rows(m_pStmt));
}

void FxMySQLStmt::Release()
{
	mysql_stmt_close(m_pStmt);
	delete this;
}

