#include "RedisConn.h"
#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif // WIN32

#ifdef WIN32
#define strtoll  _strtoi64
#define strtoull _strtoui64
#endif

// -----------------------------------------------------------------------------------------
redisconn* redisconn::Instance()
{
	static redisconn __s;
	return &__s;
}

//redisconn::redisconn(const std::string& server, unsigned int port, bool cluster)
//	: host(server), port(port), redis_cluster(cluster)
//{
//	c = NULL;
//	reply = NULL;
//	//_key = stRedisConfig::key(host, port);
//}

redisconn::redisconn()
{
	c = NULL; reply = NULL; port = 0; redis_cluster = false;
}

redisconn::~redisconn()
{
	this->disconn();
}

bool redisconn::reconnect()
{
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	this->disconn();
	
	this->c = redisConnectWithTimeout(this->host.c_str(), this->port, timeout);
	if (c->err)
	{
		redisFree(c);
		c = NULL;
		return false;
	}

	return true;
}

bool redisconn::conn(const std::string& _host, unsigned int _port)
{
	host = _host; port = _port;
	//_key = stRedisConfig::key(host, port);
	return reconnect();
}

bool redisconn::disconn()
{
	if (c)
	{
		if (reply)
		{
			freeReplyObject(reply);
			reply = NULL;
		}
		redisFree(c);
		c = NULL;
	}
	return true;
}

bool redisconn::getHandle()
{
	if (!c)
	{
		CHECK_CONNECT_VALID
		//LOG_INFO("[redisconn] reconnect redis:%s,%d", this->host.c_str(), this->port);
	}
	if (!c)
	{
		//LOG_ERROR("[redisconn] reconnect fail, redis:%s,%d", this->host.c_str(), this->port);
		return false;
	}
	HANDLE_REPLY_FREE
	return true;
}

bool redisconn::exec(const char* cmd)
{
	if (getHandle())
	{
		reply = (redisReply*)redisCommand(c, cmd);
	}
	PROCESS_REPLY_ERROR
}

//RedisVar redisconn::getInt(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%llu", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s", table);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::getInt(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s", table);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//bool redisconn::setInt(const char* table, unsigned long long id, unsigned long long value, unsigned int secs)
//{
//	if (getHandle())
//	{
//		if (secs)
//			reply = (redisReply*)redisCommand(c, "SETEX %s_%llu %u %llu", table, id, secs, value);
//		else
//			reply = (redisReply*)redisCommand(c, "SET %s_%llu %llu", table, id, value);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::setInt(const char* table, const char* id, unsigned long long value, unsigned int secs)
//{
//	if (getHandle())
//	{
//		if (secs)
//		{
//			if (id)
//			{
//				reply = (redisReply*)redisCommand(c, "SETEX %s_%s %u %llu", table, id, secs, value);
//			}
//			else
//			{
//				reply = (redisReply*)redisCommand(c, "SETEX %s %u %llu", table, secs, value);
//			}
//		}
//		else
//		{
//			if (id)
//			{
//				reply = (redisReply*)redisCommand(c, "SET %s_%s %llu", table, id, value);
//			}
//			else
//			{
//				reply = (redisReply*)redisCommand(c, "SET %s %llu", table, value);
//			}
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//RedisVar redisconn::incrInt(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "INCR %s_%llu", table, id);
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::incrInt(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "INCR %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "INCR %s", table);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::decrInt(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "DECR %s_%llu", table, id);
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::decrInt(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "DECR %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "DECR %s", table);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::getString(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "GET %s_%llu", table, id);	
//	}
//	return getValue(GET_STRING);
//}
//
//RedisVar redisconn::getString(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s", table);
//		}
//	}
//	return getValue(GET_STRING);
//}
//
//bool redisconn::setString(const char* table, unsigned long long id, const char* value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SET %s_%llu %s", table, id, value);	
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::setString(const char* table, const char* id, const char* value)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "SET %s_%s %s", table, id, value);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "SET %s %s", table, value);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//int redisconn::getBin(const char* table, unsigned long long id, RedisVar& var)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "GET %s_%llu", table, id);
//		if (!reply) { disconn(); return 0; }
//
//		if (reply->type == REDIS_REPLY_STRING)
//		{
//			var.val_bin.resize(reply->len);
//			memcpy(&var.val_bin[0], reply->str, reply->len);
//			var.val_pstr = &var.val_bin[0];
//			var.setValid(true);
//
//			freeReplyObject(reply);
//			reply = NULL;
//		}
//	}
//	return var.getBinSize();
//}
//
//int redisconn::getBin(const char* table, const char* id, RedisVar& var)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s", table);
//		}
//
//		if (!reply) { disconn(); return 0; }
//
//		if (reply->type == REDIS_REPLY_STRING)
//		{
//			var.val_bin.resize(reply->len);
//			memcpy(&var.val_bin[0], reply->str, reply->len);
//			var.val_pstr = &var.val_bin[0];
//			var.setValid(true);
//
//			freeReplyObject(reply);
//			reply = NULL;
//		}
//	}
//	return var.getBinSize();
//}
//
//int redisconn::getBin(const char* table, unsigned long long id, char* buff, int len)
//{
//	int real_len = 0;
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "GET %s_%llu", table, id);
//		if (!reply) { disconn(); return real_len; }
//
//		if (reply->type == REDIS_REPLY_STRING)
//		{
//			if (len >= reply->len)
//			{
//				real_len = reply->len;
//				memcpy(buff, reply->str, reply->len);
//			}
//			else
//			{
//				real_len = len;
//				memcpy(buff, reply->str, len);
//				LOG_ERROR("%s %d %s  buff len < reply->len ", __FILE__, __LINE__, __FUNCTION__);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//		}
//	}
//	return real_len;
//}
//
//int redisconn::getBin(const char* table, const char* id, char* buff, int len)
//{
//	int real_len = 0;
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s", table);
//		}
//
//		if (!reply) { disconn(); return real_len; }
//
//		if (reply->type == REDIS_REPLY_STRING)
//		{
//			if (len >= reply->len)
//			{
//				real_len = reply->len;
//				memcpy(buff, reply->str, reply->len);
//			}
//			else
//			{
//				real_len = len;
//				memcpy(buff, reply->str, len);
//				LOG_ERROR("%s %d %s  buff len < reply->len ", __FILE__, __LINE__, __FUNCTION__);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//		}
//	}
//	return real_len;
//}
//
//bool redisconn::setBin(const char* table, unsigned long long id, const char* value, size_t len, unsigned int secs)
//{
//	if (getHandle())
//	{
//		if (secs)
//			reply = (redisReply*)redisCommand(c, "SETEX %s_%llu %u %b", table, id, secs, value, len);
//		else
//			reply = (redisReply*)redisCommand(c, "SET %s_%llu %b", table, id, value, len);
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::setBin(const char* table, const char* id, const char* value, size_t len, unsigned int secs)
//{
//	if (getHandle())
//	{
//		if (secs)
//		{
//			if (id)
//			{
//				reply = (redisReply*)redisCommand(c, "SETEX %s_%s %u %b", table, id, secs, value, len);
//			}
//			else
//			{
//				reply = (redisReply*)redisCommand(c, "SETEX %s %u %b", table, secs, value, len);
//			}
//		}
//		else
//		{
//			if (id)
//			{
//				reply = (redisReply*)redisCommand(c, "SET %s_%s %b", table, id, value, len);
//			}
//			else
//			{
//				reply = (redisReply*)redisCommand(c, "SET %s %b", table, value, len);
//			}
//		}
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::setSet(const char* table, unsigned long long id, unsigned long long value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SADD %s_%llu %llu", table, id, value);
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::setSet(const char* table, unsigned long long id, const char* value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SADD %s_%llu %s", table, id, value);
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::setSet(const char* table, const char* id, unsigned long long value)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "SADD %s_%s %llu", table, id, value);
//		else
//			reply = (redisReply*)redisCommand(c, "SADD %s %llu", table, value);
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::setSet(const char* table, const char* id, const char* value)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "SADD %s_%s %s", table, id, value);
//		else
//			reply = (redisReply*)redisCommand(c, "SADD %s %s", table, value);
//		PROCESS_REPLY_ERROR
//	}
//	return false;
//}
//
//bool redisconn::getSet(const char* table, unsigned long long id, std::set<std::string>& valueset)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SMEMBERS %s_%llu", table, id);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int j = 0; j < reply->elements; j++)
//			{
//				if (reply->element[j]->str != NULL)
//					valueset.insert(reply->element[j]->str);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getSet(const char* table, unsigned long long id, std::set<unsigned long long>& valueset)
//{
//	std::set<std::string> __set;
//	if (getSet(table, id, __set))
//	{
//		for (std::set<std::string>::iterator it = __set.begin(); it != __set.end(); it++)
//		{
//			valueset.insert(strtoull((*it).c_str(), NULL, 10));
//		}
//		return true;
//	}
//	return false;
//}
//
//bool redisconn::getSet(const char* table, const char* id, std::set<std::string>& valueset)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "SMEMBERS %s_%s", table, id);
//		else
//			reply = (redisReply*)redisCommand(c, "SMEMBERS %s", table);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int j = 0; j < reply->elements; j++)
//			{
//				if (reply->element[j]->str != NULL)
//					valueset.insert(reply->element[j]->str);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getSet(const char* table, const char* id, std::set<unsigned long long>& valueset)
//{
//	std::set<std::string> __set;
//	if (getSet(table, id, __set))
//	{
//		for (std::set<std::string>::iterator it = __set.begin(); it != __set.end(); it++)
//		{
//			valueset.insert(strtoull((*it).c_str(), NULL, 10));
//		}
//		return true;
//	}
//	return false;
//}
//
//bool redisconn::checkSet(const char* table, unsigned long long id, const unsigned long long value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SISMEMBER %s_%llu %llu", table, id, value);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::checkSet(const char* table, unsigned long long id, const char* value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SISMEMBER %s_%llu %s", table, id, value);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::checkSet(const char* table, const char* id, const unsigned long long value)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "SISMEMBER %s_%s %llu", table, id, value);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "SISMEMBER %s %llu", table, value);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::checkSet(const char* table, const char* id, const char* value)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "SISMEMBER %s_%s %s", table, id, value);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "SISMEMBER %s %s", table, value);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delSet(const char* table, unsigned long long id, const unsigned long long value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SREM %s_%llu %llu", table, id, value);	
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delSet(const char* table, const char* id, const unsigned long long value)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "SREM %s_%s %llu", table, id, value);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "SREM %s %llu", table, value);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delSet(const char* table, const char* id, const char* value)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "SREM %s_%s %s", table, id, value);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "SREM %s %s", table, value);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delSet(const char* table, unsigned long long id, const char* value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "SREM %s_%llu %s", table, id, value);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::setZSet(const char* table, const char* id, const unsigned long long score, const char* member)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZADD %s_%s %llu %s", table, id, score, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZADD %s %llu %s", table, score, member);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delZSet(const char*table, const char* id, const char *member)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZREM %s_%s %s", table, id, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZREM %s %s", table, member);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::setZSet(const char* table, const char* id, unsigned long long score, unsigned long long member)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZADD %s_%s %llu %llu", table, id, score, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZADD %s %llu %llu", table, score, member);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delZSet(const char* table, const char* id, unsigned long long member)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZREM %s_%s %llu", table, id, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZREM %s %llu", table, member);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delZSetByRank(const char* table, const char* id, int start, int end)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZREMRANGEBYRANK %s_%s %d %d", table, id, start, end);
//		else
//			reply = (redisReply*)redisCommand(c, "ZREMRANGEBYRANK %s %d %d", table, start, end);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::delZSetByScore(const char* table, const char* id, long long start, long long end)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZREMRANGEBYSCORE %s_%s %lld %lld", table, id, start, end);
//		else
//			reply = (redisReply*)redisCommand(c, "ZREMRANGEBYSCORE %s %lld %lld", table, start, end);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//RedisVar redisconn::addZSetScore(const char*table, const char* id, const char *member, const long long score)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZINCRBY %s_%s %lld %s", table, id, score, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZINCRBY %s %lld %s", table, score, member);
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::getZScore(const char* table, const char* id, const char* member)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZSCORE %s_%s %s", table, id, member);
//		else
//			reply = (redisReply*)redisCommand(c, "ZSCORE %s %s", table, member);
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::getZRank(const char* table, const char* id, const char* member, bool reverse)
//{
//	if (getHandle())
//	{
//		if (reverse)
//		{
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANK %s_%s %s", table, id, member);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANK %s %s", table, member);
//		}
//		else
//		{
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANK %s_%s %s", table, id, member);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANK %s %s", table, member);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//bool redisconn::getZSetByRank(const char* table, const char* id, std::vector<std::string>& valueset, int start, int end, bool reverse)
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s_%s %d %d", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s %d %d", table, start, end);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s_%s %d %d", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s %d %d", table, start, end);
//		if (!reply){ disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int j = 0; j < reply->elements; j++)
//			{
//				if (reply->element[j]->str != NULL)
//					valueset.push_back(reply->element[j]->str);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getZSetByRank(const char* table, const char* id, std::set<std::string>& valueset, int start, int end, bool reverse /*= false*/)
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s_%s %d %d", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s %d %d", table, start, end);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s_%s %d %d", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s %d %d", table, start, end);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int j = 0; j < reply->elements; j++)
//			{
//				if (reply->element[j]->str != NULL)
//					valueset.insert(reply->element[j]->str);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getZSetByRankWithScore(const char* table, const char* id, std::map<std::string, unsigned long long>& valuemap, int start, int end, bool reverse)
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s_%s %d %d WITHSCORES", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGE %s %d %d WITHSCORES", table, start, end);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s_%s %d %d WITHSCORES", table, id, start, end);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGE %s %d %d WITHSCORES", table, start, end);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int key = 0; key < reply->elements; key += 2)
//			{
//				unsigned int value = key + 1;
//				if (value < reply->elements)
//				{
//					if (reply->element[value]->str != NULL)
//						valuemap.insert(std::make_pair(reply->element[key]->str, atoi(reply->element[value]->str)));
//				}
//				else
//				{
//					break;
//				}
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getZSetByScore(const char* table, const char* id, std::vector<std::string>& valuevec, long long start, long long end, int offset, int count, bool reverse)
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s %lld %lld LIMIT %d %d", table, start, end, offset, count);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s %lld %lld LIMIT %d %d", table, start, end, offset, count);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int j = 0; j < reply->elements; j++)
//			{
//				if (reply->element[j]->str != NULL)
//					valuevec.push_back(reply->element[j]->str);
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//
//bool redisconn::getZSetByScoreWithScore(const char* table, const char* id, std::map<std::string, unsigned long long>& valuemap, long long start, long long end, int offset, int count, bool reverse)
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d WITHSCORES", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s %lld %lld LIMIT %d %d WITHSCORES", table, start, end, offset, count);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d WITHSCORES", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s %lld %lld LIMIT %d %d WITHSCORES", table, start, end, offset, count);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int key = 0; key < reply->elements; key += 2)
//			{
//				unsigned int value = key + 1;
//				if (value < reply->elements)
//				{
//					if (reply->element[value]->str != NULL)
//						valuemap.insert(std::make_pair(reply->element[key]->str, strtoull(reply->element[value]->str, NULL, 10)));
//				}
//				else
//				{
//					break;
//				}
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//bool redisconn::getZSetBinByScoreWithScore(const char* table, const char* id, std::vector<RedisVar>& ret, long long start, long long end, int offset , int count , bool reverse )
//{
//	if (getHandle())
//	{
//		if (reverse)
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d WITHSCORES", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZREVRANGEBYSCORE %s %lld %lld LIMIT %d %d WITHSCORES", table, start, end, offset, count);
//		else
//			if (id)
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s_%s %lld %lld LIMIT %d %d WITHSCORES", table, id, start, end, offset, count);
//			else
//				reply = (redisReply*)redisCommand(c, "ZRANGEBYSCORE %s %lld %lld LIMIT %d %d WITHSCORES", table, start, end, offset, count);
//		if (!reply) { disconn(); return false; }
//
//		if (reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int key = 0; key < reply->elements; key += 2)
//			{
//				unsigned int value = key + 1;
//				if (value < reply->elements)
//				{
//					RedisVar vr;
//
//					vr.val_bin.resize(reply->element[key]->len);
//					memset(&vr.val_bin[0], 0, reply->element[key]->len);
//					memcpy(&vr.val_bin[0], reply->element[key]->str, reply->element[key]->len);
//					vr.val_pstr = &vr.val_bin[0];
//					vr.setValid(true);
//					vr.val_ulonglong = strtoull(reply->element[value]->str, NULL, 10);
//					ret.push_back(vr);
//				}
//				else
//				{
//					break;
//				}
//			}
//			freeReplyObject(reply);
//			reply = NULL;
//			return true;
//		}
//		freeReplyObject(reply);
//		reply = NULL;
//	}
//	return false;
//}
//unsigned int redisconn::getZSetCount(const char* table, const char* id, unsigned long long start, unsigned long long end)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZCOUNT %s_%s %llu %llu", table, id, start, end);
//		else
//			reply = (redisReply*)redisCommand(c, "ZCOUNT %s %llu %llu", table, start, end);
//	}
//	return getValue(GET_INTEGER);
//}
//
//bool redisconn::setZSetBin(const char* table, const char* id, const unsigned long long score, const char* member, size_t len)
//{
//	if (getHandle())
//	{
//		if (id)
//			reply = (redisReply*)redisCommand(c, "ZADD %s_%s %llu %b", table, id, score, member, len);
//		else
//			reply = (redisReply*)redisCommand(c, "ZADD %s %llu %b", table, score, member, len);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//
//unsigned int redisconn::mset(const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "mset %s", pattern);
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::mget(std::vector<RedisVar>& ret, const char* pattern, ...)
//{
//	if (redis_cluster == true)
//	{
//		JyAst(false);
//		return 0;
//	}
//
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "mget %s", pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//
//	if (!reply) { disconn(); return 0; }
//	unsigned int elements = 0;
//
//	switch (reply->type)
//	{
//	case REDIS_REPLY_ARRAY:
//		{
//			//LOG_INFO("reply->elements:%lu", reply->elements);
//			elements = reply->elements;
//			for (unsigned int i = 0; i<reply->elements; i++)
//			{
//				//LOG_INFO("reply->elements%d:type:%d,value:%s", i, reply->element[i]->type, reply->element[i]->str);
//				RedisVar vr;
//				if (reply->element[i]->str != NULL)
//				{
//					vr.val_bin.resize(reply->element[i]->len);
//					memset(&vr.val_bin[0], 0, reply->element[i]->len);
//					memcpy(&vr.val_bin[0], reply->element[i]->str, reply->element[i]->len);
//					vr.val_pstr = &vr.val_bin[0];
//					vr.setValid(true);
//				}
//				ret.push_back(vr);
//			}
//		}
//		break;
//	case REDIS_REPLY_STATUS:
//	case REDIS_REPLY_ERROR:
//	case REDIS_REPLY_NIL:
//	case REDIS_REPLY_STRING:
//	case REDIS_REPLY_INTEGER:
//	default:
//		break;
//	}
//
//	freeReplyObject(reply);
//	reply = NULL;
//	return elements;
//}
//
//unsigned int redisconn::mget(const char* table, std::vector<RedisVar>& ret, std::vector<unsigned long long>& ids)
//{
//	if (redis_cluster == false)
//	{
//		std::ostringstream keys;
//		for (std::vector<unsigned long long>::iterator it = ids.begin(); it != ids.end(); it++)
//		{
//			keys << " " << table << "_" << *it << "";//%s_%llu
//		}
//		return mget(ret, keys.str().c_str());
//	}
//	for (std::vector<unsigned long long>::iterator it = ids.begin(); it != ids.end(); it++)
//	{
//		RedisVar var;
//		getBin(table, *it, var);
//		ret.push_back(var);
//	}
//	return ids.size();
//}
//
//unsigned int redisconn::mget(const char* table, std::vector<RedisVar>& ret, std::vector<std::string>& ids)
//{
//	if (redis_cluster == false)
//	{
//		std::ostringstream keys;
//		for (std::vector<std::string>::iterator it = ids.begin(); it != ids.end(); it++)
//		{
//			keys << " " << table << "_" << *it << "";//%s_%s
//		}
//		return mget(ret, keys.str().c_str());
//	}
//	for (std::vector<std::string>::iterator it = ids.begin(); it != ids.end(); it++)
//	{
//		RedisVar var;
//		getBin(table, (*it).c_str(), var);
//		ret.push_back(var);
//	}
//	return ids.size();
//}
//
//unsigned int redisconn::mget(const char* table, std::map<unsigned long long, RedisVar>& valuemap)
//{
//	if (redis_cluster == false)
//	{
//		std::vector<RedisVar> ret;
//		std::vector<unsigned long long> ids;
//		for (std::map<unsigned long long, RedisVar>::iterator it = valuemap.begin(); it != valuemap.end(); it++)
//		{
//			ids.push_back(it->first);
//		}
//		mget(table, ret, ids);
//		for (unsigned int i = 0; i < ret.size(); i++)
//		{
//			valuemap[ids[i]] = ret[i];
//		}
//		return ret.size();
//	}
//	for (std::map<unsigned long long, RedisVar>::iterator it = valuemap.begin(); it != valuemap.end(); it++)
//	{
//		RedisVar var;
//		getBin(table, it->first, it->second);
//	}
//	return valuemap.size();
//}
//
//unsigned int redisconn::mget(const char* table, std::map<std::string, RedisVar>& valuemap)
//{
//	if (redis_cluster == false)
//	{
//		std::vector<RedisVar> ret;
//		std::vector<string> ids;
//		for (std::map<std::string, RedisVar>::iterator it = valuemap.begin(); it != valuemap.end(); it++)
//		{
//			ids.push_back(it->first);
//		}
//		mget(table, ret, ids);
//		for (unsigned int i = 0; i < ret.size(); i++)
//		{
//			valuemap[ids[i]] = ret[i];
//		}
//		return ret.size();
//	}
//	for (std::map<std::string, RedisVar>::iterator it = valuemap.begin(); it != valuemap.end(); it++)
//	{
//		RedisVar var;
//		getBin(table, it->first.c_str(), it->second);
//	}
//	return valuemap.size();
//}
//
//unsigned int redisconn::hmset(const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "hmset %s", pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::hmset(const char* szTable, const char* szId, const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "hmset %s_%s %s", szTable, szId, pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::hmset(const char* szTable, const unsigned long long Id, const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "hmset %s_%llu %s", szTable, Id, pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::hmset(const char* szTable, const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "hmset %s %s", szTable, pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::hset(const char* szTable, const char* szField, const char* value)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "HSET %s %s %s", szTable, szField, value);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//unsigned int redisconn::hmget(std::vector<RedisVar>& ret, const char* pattern, ...)
//{
//	char buf[20 * 1024] = { 0 };
//	sprintf(buf, "hmget %s", pattern);
//
//	va_list ap;
//	if (getHandle())
//	{
//		va_start(ap, pattern);
//		reply = (redisReply*)redisvCommand(c, buf, ap);
//		va_end(ap);
//	}
//
//	if (!reply) { disconn(); return 0; }
//	unsigned int elements = 0;
//
//	switch (reply->type)
//	{
//	case REDIS_REPLY_ARRAY:
//		{
//			//LOG_INFO("reply->elements:%lu", reply->elements);
//			elements = reply->elements;
//			for (unsigned int i = 0; i<reply->elements; i++)
//			{
//				//LOG_INFO("reply->elements%d:type:%d,value:%s", i, reply->element[i]->type, reply->element[i]->str);
//				RedisVar vr;
//				if (reply->element[i]->str != NULL)
//				{			
//					if (reply->element[i]->type == REDIS_REPLY_INTEGER)
//					{
//						vr.val_ulonglong = reply->integer;
//						vr.setValid(true);
//					}			
//					vr.val_bin.resize(reply->element[i]->len + 1);
//					memset(&vr.val_bin[0], 0, reply->element[i]->len + 1);
//					memcpy(&vr.val_bin[0], reply->element[i]->str, reply->element[i]->len);
//					vr.val_pstr = &vr.val_bin[0];
//					vr.setValid(true);
//				}
//				ret.push_back(vr);
//			}
//		}
//		break;
//	case REDIS_REPLY_STATUS:
//	case REDIS_REPLY_ERROR:
//	case REDIS_REPLY_NIL:
//	case REDIS_REPLY_STRING:
//	case REDIS_REPLY_INTEGER:
//	default:
//		break;
//	}
//
//	freeReplyObject(reply);
//	reply = NULL;
//	return elements;
//}
//
//unsigned int redisconn::hmget(const char* szTable, const char* szId, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret)
//{
//	std::ostringstream sstrRedis;
//	sstrRedis << szTable << "_" << szId;
//
//	for (std::vector<std::string>::const_iterator cit = vecstrColumn.begin(); cit != vecstrColumn.end(); ++cit)
//	{
//		sstrRedis << " " << *cit;
//	}
//	return hmget(ret, sstrRedis.str().c_str());
//}
//
//unsigned int redisconn::hmget(const char* szTable, const char* szId, const char* strColumn, std::vector<RedisVar>& ret)
//{
//	std::ostringstream sstrRedis;
//	sstrRedis << szTable << "_" << szId << " " << strColumn;
//	return hmget(ret, sstrRedis.str().c_str());
//}
//
//unsigned int redisconn::hmget(const char* szTable, unsigned long long dwpId, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret)
//{
//	char buff[64] = { 0 };
//	sprintf(buff, "%llu", dwpId);
//	return hmget(szTable, buff, vecstrColumn, ret);
//}
//
//unsigned int redisconn::hmget(const char* szTable, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret)
//{
//	std::ostringstream sstrRedis;
//	sstrRedis << szTable;
//
//	for (std::vector<std::string>::const_iterator cit = vecstrColumn.begin(); cit != vecstrColumn.end(); ++cit)
//	{
//		sstrRedis << " " << *cit;
//	}
//	return hmget(ret, sstrRedis.str().c_str());
//}
//
//unsigned int redisconn::hmget(const char* szTable, unsigned long long dwpId, const char* strColumn, std::vector<RedisVar>& ret)
//{
//	char buff[64] = { 0 };
//	sprintf(buff, "%llu", dwpId);
//	return hmget(szTable, buff, strColumn, ret);
//}
//
//unsigned int redisconn::hmget(const char* szTable, const char* strColumn, std::vector<RedisVar>& ret)
//{
//	std::ostringstream sstrRedis;
//	sstrRedis << szTable << " " << strColumn;
//	return hmget(ret, sstrRedis.str().c_str());
//}
//
//bool redisconn::hdel(const char* szTable, const char* Id, const char* szFields)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "HDEL %s_%s %s", szTable, Id, szFields);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//RedisVar redisconn::hgetInt(const char* table, const char* id, const char* field)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s_%s %s", table, id, field);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s %s", table, field);
//		}
//	}
//	return getValue(GET_INTEGER);
//}
//
//RedisVar redisconn::hgetString(const char* table, const char* id, const char* field)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s_%s %s", table, id, field);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s %s", table, field);
//		}
//	}
//	return getValue(GET_STRING);
//}
//
//RedisVar redisconn::hgetBin(const char* table, const char* id, const char* field)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s_%s %s", table, id, field);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "hget %s %s", table, field);
//		}
//	}
//	return getValue(GET_BIN);
//}
//
//bool redisconn::del(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "DEL %s_%llu", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "DEL %s", table);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::del(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "DEL %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "DEL %s", table);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::exist(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "EXISTS %s_%llu", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "EXISTS %s", table);
//		}
//	}
//	return (int)getValue(GET_INTEGER) > 0;
//}
//
//bool redisconn::exist(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "EXISTS %s_%s", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "EXISTS %s", table);
//		}
//	}
//	return (int)getValue(GET_INTEGER) > 0;
//}
//
//bool redisconn::watch(const char* table, unsigned long long id)
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "WATCH %s_%llu_cas_lock_", table, id);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::watch(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "WATCH %s_%s_cas_lock_", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "WATCH %s_cas_lock_", table);
//		}
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::unwatch()
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "UNWATCH");
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::lock()
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "MULTI");
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::unlock()
//{
//	if (getHandle())
//	{
//		reply = (redisReply*)redisCommand(c, "EXEC");
//	}
//	if (reply == NULL)
//	{
//		//LOG_INFO("redisconn::unlock reply == NULL ");
//		this->disconn();
//		return false;
//	}
//
//	if (reply->type == REDIS_REPLY_NIL || reply->type == REDIS_REPLY_ERROR)
//	{
//		//LOG_INFO("redisconn::unlock reply->type = %d ", reply->type);
//		freeReplyObject(reply);
//		reply = NULL;
//		return false;
//	}
//
//	freeReplyObject(reply);
//	reply = NULL;
//	return true;
//}
//
//bool redisconn::islockedtable(const char* table, const char* id)
//{
//	if (getHandle())
//	{
//		if (id)
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_%s_cas_lock_", table, id);
//		}
//		else
//		{
//			reply = (redisReply*)redisCommand(c, "GET %s_cas_lock_", table);
//		}
//		RedisVar lock_flag = getValue(GET_INTEGER);
//		if (lock_flag.isValid() && (int)lock_flag == 1)
//			return true;
//		return false;
//	}
//	return true;
//}
//
//bool redisconn::locktable(const char* table, const char* id, int block, unsigned int secs)
//{
//	if (!getHandle()) return false;
//	
//	int i = 0;
//	while (true)
//	{
//		if (!watch(table, id)) return false;
//		if (!islockedtable(table, id)) break;
//		unwatch();
//		i++;
//		if (i > block) return false;
//		//LOG_INFO("redisconn::locktable %d sleep ", i);
//		JySleep(1);
//	}
//	
//	if (!this->lock()) { unwatch(); return false; }
//
//	if (id)
//	{
//		if (secs == 0)
//			reply = (redisReply*)redisCommand(c, "SET %s_%s_cas_lock_ 1", table, id);
//		else
//			reply = (redisReply*)redisCommand(c, "SETEX %s_%s_cas_lock_ %u 1", table, id, secs);
//	}
//	else
//	{
//		if (secs == 0)
//			reply = (redisReply*)redisCommand(c, "SET %s_cas_lock_ 1", table);
//		else
//			reply = (redisReply*)redisCommand(c, "SETEX %s_cas_lock_ %u 1", table, secs);
//	}
//	if (!reply){ unwatch(); disconn(); return false; }
//
//	freeReplyObject(reply);
//	reply = NULL;
//	return unlock();
//}
//
//bool redisconn::locktable(const char* table, const char* id, int loop, int block, unsigned int secs)
//{
//	while (loop > 0) {
//		if (locktable(table, id, block, secs)) return true;
//		loop--;
//	}
//	return false;
//}
//
//bool redisconn::unlocktable(const char* table, const char* id, bool del)
//{
//	if (del) return dellocktable(table, id);
//	if (id)
//	{
//		reply = (redisReply*)redisCommand(c, "SET %s_%s_cas_lock_ 0", table, id);
//	}
//	else
//	{
//		reply = (redisReply*)redisCommand(c, "SET %s_cas_lock_ 0", table);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//bool redisconn::dellocktable(const char* table, const char* id)
//{
//	if (id)
//	{
//		reply = (redisReply*)redisCommand(c, "DEL %s_%s_cas_lock_ 0", table, id);
//	}
//	else
//	{
//		reply = (redisReply*)redisCommand(c, "DEL %s_cas_lock_ 0", table);
//	}
//	PROCESS_REPLY_ERROR
//}
//
//RedisVar redisconn::getValue(int type)
//{
//	RedisVar ret;
//	if (!reply) { disconn(); return ret; }
//
//	switch (reply->type)
//	{
//		case REDIS_REPLY_INTEGER:
//			if (type == GET_INTEGER) { ret.val_ulonglong = reply->integer; ret.setValid(true); }
//			break;
//		case REDIS_REPLY_STRING:
//			if (type == GET_INTEGER) { ret.val_ulonglong = strtoull(reply->str, NULL, 10); ret.setValid(true); }
//			else if (type == GET_STRING || GET_BIN)
//			{
//				ret.val_bin.resize(reply->len + 1);
//				memset(&ret.val_bin[0], 0, reply->len + 1);
//				memcpy(&ret.val_bin[0], reply->str, reply->len);
//				ret.val_pstr = &ret.val_bin[0];
//				ret.setValid(true);
//			}
//			break;
//		case REDIS_REPLY_STATUS:
//		case REDIS_REPLY_ERROR:
//		case REDIS_REPLY_NIL:
//		case REDIS_REPLY_ARRAY:
//		default:
//			break;
//	}
//
//	freeReplyObject(reply);
//	reply = NULL;
//	return ret;
//}
//
//bool redisconn::getKeys(const char* key, std::vector<std::string> &vec_keys)
//{
//	vec_keys.clear();
//	if (key)
//	{
//		reply = (redisReply*)redisCommand(c, "KEYS %s", key);
//		if (reply && reply->type == REDIS_REPLY_ARRAY)
//		{
//			for (unsigned int key = 0; key < reply->elements; key ++)
//			{
//				vec_keys.push_back(reply->element[key]->str);
//			}
//		}
//	}
//
//	freeReplyObject(reply);
//	reply = NULL;
//	if (vec_keys.size() > 0)
//		return true;
//	return false;
//}
// -----------------------------------------------------------------------------------------
redispool* redispool::Instance()
{
	static redispool __s;
	return &__s;
}

redispool::~redispool()
{
	for (REDISDBMAP::iterator i = m_dbpool.begin(); i != m_dbpool.end(); i++)
	{
		REDISDBVEC& v = i->second;
		for (REDISDBVEC::iterator it = v.begin(); it != v.end(); it++)
		{
			delete (*it);
		}
	}
	m_dbpool.clear();
	m_dbs.clear();
	m_threadconn.clear();
}

void redispool::init(REDISDBURL& dburls)
{
	for (REDISDBURL::iterator it = dburls.begin(); it != dburls.end(); it++)
	{
		stRedisConfig& cfg = (*it);
		m_dburls[cfg.key(cfg.m_sIP, cfg.m_uiPort)] = cfg;
	}

	for (REDISDBURLMAP::iterator it = m_dburls.begin(); it != m_dburls.end(); it++)
	{
		stRedisConfig& cfg = it->second;
		redisconn* conn = NULL;
		for (unsigned short i = 0; i < cfg.m_uiThreadNum; i++)
		{
			conn = new redisconn;// (cfg.m_sIP, cfg.m_uiPort, cfg.m_bCluster);
			m_dbpool[conn->key()].push_back(conn);
			m_dbs.push_back(conn);
			if (conn->reconnect() == false)
			{
				//LOG_ERROR("%s %d %s", __FILE__, __LINE__, __FUNCTION__);
				//LOG_ERROR("Redis（%s），端口%d 无法连接", cfg.m_sIP.c_str(), cfg.m_uiPort);
				//JyAst(false);
			}
		}
	}
}

void redispool::exit()
{
	for (REDISDBMAP::iterator i = m_dbpool.begin(); i != m_dbpool.end(); i++)
	{
		REDISDBVEC& v = i->second;
		for (REDISDBVEC::iterator it = v.begin(); it != v.end(); it++)
		{
			(*it)->disconn();
		}
	}
}

void redispool::flush()
{
	for (REDISDBMAP::iterator i = m_dbpool.begin(); i != m_dbpool.end(); i++)
	{
		if (m_dburls[i->first].m_bFlush)
		{
			(*(i->second.begin()))->exec("FLUSHALL");
		}
	}
}

redisconn* redispool::__create()
{
	//LOG_INFO("%s %d %s", __FILE__, __LINE__, __FUNCTION__);
	static std::vector<stRedisConfig*> __v;
	if (m_dbs.size())
	{
		redisconn* conn = *m_dbs.begin();
		m_dbs.erase(m_dbs.begin());
		return conn;
	}

	if (__v.size() == 0)
	{
		for (REDISDBURLMAP::iterator it = m_dburls.begin(); it != m_dburls.end(); it++)
		{
			__v.push_back(&(it->second));
		}
	}

	if (__v.size() == 0)
		return NULL;

	unsigned int i = 0;// RandInt(0, __v.size());
	stRedisConfig* cfg = __v[i];
	redisconn* conn = new redisconn;// (cfg->m_sIP, cfg->m_uiPort, cfg->m_bCluster);
	m_dbpool[conn->key()].push_back(conn);
	//m_dbs.push_back(conn);

	if (conn->reconnect() == false)
	{
		//LOG_ERROR("%s %d %s", __FILE__, __LINE__, __FUNCTION__);
		//LOG_ERROR("Redis（%s），端口%d 无法连接", cfg->m_sIP.c_str(), cfg->m_uiPort);
	}
	return conn;
}
// -----------------------------------------------------------------------------------------