#ifndef __DBREADER_H_2009_0824__
#define __DBREADER_H_2009_0824__

#include "fxredis.h"
#include "hiredis.h"

class FxRedisReader : public IRedisDataReader
{
	friend class FxRedisConnection;
public:
	FxRedisReader();
	virtual				~FxRedisReader();

	virtual bool		GetValue(std::string& refszValue);
	virtual bool		GetValue(std::vector<std::string>& refvecszValue);
	virtual bool		GetValue(std::vector<long long>& refvecqwValue);
	virtual bool		GetValue(long long& refqwValue);

	virtual void        Release();

protected:
	redisReply* reply;
};

#endif  // __DBREADER_H_2009_0824__

