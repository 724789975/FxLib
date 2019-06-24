#ifndef __DBCLIENT_H_2009_0824__
#define __DBCLIENT_H_2009_0824__

#include <list>
#include <time.h>
#include <fxmeta.h>
#include "dbconnection.h"
#include "lock.h"

class FxMySqlClient : public IDBConnection, public IFxThread
{
public:
	FxMySqlClient();
	virtual ~FxMySqlClient();

	virtual unsigned int		GetLastError(void);
	virtual const char* GetLastErrorString(void);
	virtual unsigned int		EscapeString(const char* pszSrc, int nSrcLen, char* pszDest, int nDestLen);
	virtual int		Query(const char* pszSQL);
	virtual int		Query(const char* pszSQL, IDataReader **ppReader);
	virtual void		ThrdFunc();
	virtual void		Stop();
	virtual IStmt*		CreateStmt();

	bool		        Start();
	bool		        ConnectDB(SDBAccount& account);
	bool		        AddQuery(IQuery* poQuery);
	
private:
	void			    __ClearQuery();
	bool		        __DoQuery();
	void		        __Reset();

private:
	FxCriticalLock			m_oLock;
	IFxThreadHandler*	m_poThrdHandler;
	FxMySQLConnection*	m_poMySqlConn;		
	std::list<IQuery*>		 m_oQuerys;
	time_t				m_nLastReconnectTime;
	bool				m_bTerminate;
	bool				m_bDbOK;

	FILE*				m_pFile;
	char				m_szLogPath[64];
};

#endif	// __DBCLIENT_H_2009_0824__

