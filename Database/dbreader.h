#ifndef __DBREADER_H_2009_0824__
#define __DBREADER_H_2009_0824__

#include "fxdb.h"

class FxMySQLReader : public IDataReader
{
	friend class FxMySQLConnection;
public:
	FxMySQLReader();
	virtual ~FxMySQLReader();

	virtual UINT32      GetRecordCount();

	virtual UINT32      GetFieldCount();

    virtual bool        GetFirstRecord();

    virtual bool        GetNextRecord();

	virtual const char* GetFieldValue(UINT32 dwIndex);

    virtual INT32       GetFieldLength(UINT32 dwIndex);

	virtual void        Release();

protected:
	MYSQL_RES*          m_res;
	MYSQL_ROW           m_row; 
	unsigned long *	    m_adwLengths;
};

#endif  // __DBREADER_H_2009_0824__

