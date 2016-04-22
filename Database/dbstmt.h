#ifndef __DB_STMT_H_2012_11_28__
#define __DB_STMT_H_2012_11_28__

#include "fxdb.h"

class FxMySQLConnection;

class FxMySQLStmt : public IStmt
{
	friend class FxMySQLConnection;
public:
	FxMySQLStmt(FxMySQLConnection* pConnection, MYSQL_STMT* pStmt);
	~FxMySQLStmt();

	bool		Prepare(const char* pszSQL, UINT32 dwParamNum, UINT32 dwResultNum);

	void		BindParam(enum_field_types eFieldType, void* pBuf, UINT32 dwSize);

	bool		BindParamComplete();

	void		BindResult(enum_field_types eFieldType, void* pBuf, UINT32 dwBufSize, UINT32* pdwLength);

	bool		BindResultComplete();

	bool		Execute();

	const char* GetErrorString();

	UINT32		AffectedRows();

	bool		Restore();

	bool		Fetch();

	UINT32		NumOfRows();

	void		Release();

private:
	FxMySQLConnection* m_pDBConnection;
	MYSQL_STMT* m_pStmt;
	MYSQL_BIND* m_pParamsBind;
	UINT32 m_dwParamsNum;
	UINT32 m_dwCurBindParamPos;
	MYSQL_BIND* m_pResultsBind;
	UINT32 m_dwResultsNum;
	UINT32 m_dwCurBindResultPos;
};

#endif /* __DB_STMT_H_2012_11_28__ */