#ifndef __DB_STMT_H_2012_11_28__
#define __DB_STMT_H_2012_11_28__

#include "fxdb.h"
#include "mysql.h"

class FxMySQLConnection;

class FxMySQLStmt : public IStmt
{
	friend class FxMySQLConnection;
public:
	FxMySQLStmt(FxMySQLConnection* pConnection, MYSQL_STMT* pStmt);
	~FxMySQLStmt();

	bool		Prepare(const char* pszSQL, unsigned int dwParamNum, unsigned int dwResultNum);

	void		BindParam(unsigned int eFieldType, void* pBuf, unsigned int dwSize);

	bool		BindParamComplete();

	void		BindResult(unsigned int eFieldType, void* pBuf, unsigned int dwBufSize, unsigned int* pdwLength);

	bool		BindResultComplete();

	bool		Execute();

	const char* GetErrorString();

	unsigned int		AffectedRows();

	bool		Restore();

	bool		Fetch();

	unsigned int		NumOfRows();

	void		Release();

private:
	FxMySQLConnection* m_pDBConnection;
	MYSQL_STMT* m_pStmt;
	MYSQL_BIND* m_pParamsBind;
	MYSQL_BIND* m_pResultsBind;
	unsigned int m_dwCurBindParamPos;
	unsigned int m_dwParamsNum;
	unsigned int m_dwCurBindResultPos;
	unsigned int m_dwResultsNum;
};

#endif /* __DB_STMT_H_2012_11_28__ */
