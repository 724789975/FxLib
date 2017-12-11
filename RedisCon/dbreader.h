#ifndef __DBREADER_H_2009_0824__
#define __DBREADER_H_2009_0824__

#include "fxredis.h"

class FxRedisReader : public IRedisDataReader
{
	friend class FxRedisConnection;
public:
	FxRedisReader();
	virtual ~FxRedisReader();

	virtual UINT32      GetRecordCount();

	virtual UINT32      GetFieldCount();

    virtual bool        GetFirstRecord();

    virtual bool        GetNextRecord();

	virtual const char* GetFieldValue(UINT32 dwIndex);

    virtual INT32       GetFieldLength(UINT32 dwIndex);

	virtual void        Release();

protected:
	unsigned long *	    m_adwLengths;

	redisReply* reply;
};

#endif  // __DBREADER_H_2009_0824__

