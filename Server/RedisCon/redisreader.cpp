#include "redisreader.h"
#include "redismodule.h"
#include "hiredis.h"

FxRedisReader::FxRedisReader()
:reply(NULL)
{
}

FxRedisReader::~FxRedisReader()
{
	if (reply != NULL)
	{
		freeReplyObject(reply);
		reply = NULL;
	}
}

bool FxRedisReader::GetValue(std::string& refszValue)
{
	if (reply == NULL)
	{
		return false;
	}
	if (reply->type != REDIS_REPLY_STRING)
	{
		return false;
	}
	refszValue.assign(reply->str, reply->len);

	if (reply != NULL)
	{
		freeReplyObject(reply);
	}

	reply = NULL;
	return true;
}

bool FxRedisReader::GetValue(std::vector<std::string>& refvecszValue)
{
	if (reply == NULL)
	{
		return false;
	}
	if (reply->type != REDIS_REPLY_ARRAY)
	{
		return false;
	}
	for (unsigned int i = 0; i < reply->elements; i++)
	{
		if (reply->element[i]->type != REDIS_REPLY_STRING)
		{
			return false;
		}

		std::string szValue;
		szValue.assign(reply->element[i]->str, reply->element[i]->len);
		refvecszValue.push_back(szValue);
	}

	if (reply != NULL)
	{
		freeReplyObject(reply);
	}

	reply = NULL;
	return true;
}

bool FxRedisReader::GetValue(std::vector<long long>& refvecqwValue)
{
	if (reply == NULL)
	{
		return false;
	}
	if (reply->type != REDIS_REPLY_ARRAY)
	{
		return false;
	}
	for (unsigned int i = 0; i < reply->elements; i++)
	{
		if (reply->element[i]->type != REDIS_REPLY_INTEGER)
		{
			return false;
		}

		refvecqwValue.push_back((long long)(reply->element[i]->integer));
	}

	if (reply != NULL)
	{
		freeReplyObject(reply);
	}

	reply = NULL;
	return true;
}

bool FxRedisReader::GetValue(long long& refqwValue)
{
	if (reply == NULL)
	{
		return false;
	}
	if (reply->type != REDIS_REPLY_INTEGER)
	{
		return false;
	}
	refqwValue = reply->integer;

	if (reply != NULL)
	{
		freeReplyObject(reply);
	}

	reply = NULL;
	return true;
}

void FxRedisReader::Release()
{
	if (reply != NULL)
    {
		freeReplyObject(reply);
    }

	reply = NULL;

    FxRedisModule::Instance()->ReleaseReader(this);
}

