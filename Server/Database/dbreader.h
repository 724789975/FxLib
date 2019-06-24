#ifndef __DBREADER_H_2009_0824__
#define __DBREADER_H_2009_0824__

#include "fxdb.h"
#include "mysql.h"

class FxMySQLReader : public IDataReader
{
	friend class FxMySQLConnection;
public:
	FxMySQLReader();
	virtual ~FxMySQLReader();

	virtual unsigned int      GetRecordCount();

	virtual unsigned int      GetFieldCount();

    virtual bool        GetFirstRecord();

    virtual bool        GetNextRecord();

	virtual const char* GetFieldValue(unsigned int dwIndex);

    virtual int       GetFieldLength(unsigned int dwIndex);

	virtual void        Release();

protected:
	MYSQL_RES*          m_res;
	MYSQL_ROW           m_row;
	unsigned long *	    m_adwLengths;
};

#endif  // __DBREADER_H_2009_0824__

