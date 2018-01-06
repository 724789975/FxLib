#ifndef _redisconn_H_
#define _redisconn_H_
#include "hiredis.h"
#include <set>
#include <vector>
#include <map>
#include "lock.h"
// -----------------------------------------------------------------------------------------
#define PROCESS_REPLY_ERROR if(reply == NULL) { this->disconn();return false; } \
	if (reply->type == REDIS_REPLY_ERROR) \
	{ \
		freeReplyObject(reply); \
		reply = NULL; \
		return false; \
	} \
	freeReplyObject(reply); \
	reply = NULL; \
	return true; \

#define HANDLE_REPLY_FREE if(reply != NULL)  freeReplyObject(reply); \
	reply = NULL; \

#define CHECK_CONNECT_VALID if (!c) reconnect(); \

class RedisVar
{
public:
	RedisVar(int v)
	{
		val_ulonglong = v;
		val_pstr = NULL;
		//val_bin.clear();
		valid = false;
	}
	RedisVar()
	{
		reset();
	}
	void reset()
	{
		val_ulonglong = 0;
		val_pstr = NULL;
		val_bin.clear();
		valid = false;
	}
	operator unsigned short() const	{ return (unsigned short)val_ulonglong; }
	operator short() const { return (short)val_ulonglong; }
	operator int() const { return (int)val_ulonglong; }
	operator unsigned int() const { return (unsigned int)val_ulonglong; }
	operator unsigned long() const { return (unsigned long)val_ulonglong; }
	operator unsigned long long() const { return (unsigned long long)val_ulonglong; }
	operator long long() const { return (long long)val_ulonglong; }
	operator long() const { return (long)val_ulonglong; }
	operator float() const { return (float)val_ulonglong; }
	operator double() const { return (double)val_ulonglong; }
	operator unsigned char() const { return (unsigned char)val_ulonglong; }
	operator const char*() const { return val_pstr;	}
	const char* getBin() const{ return &val_bin[0]; }
	unsigned int getBinSize(){ return val_bin.size(); }
	void setValid(bool value){ valid = value;}
	bool isValid(){ return valid; }
	unsigned long long val_ulonglong;
	const char* val_pstr;
	std::vector<char> val_bin;
	bool valid;
};

const int GET_INTEGER = 1;
const int GET_STRING = 2;
const int GET_BIN = 3;

class redisconn
{
public:
	redisconn();
	//redisconn(const std::string& host, unsigned int port, bool cluster = false);
	~redisconn();
	unsigned long long key() { return _key; };
	bool reconnect();
	bool conn(const std::string& host, unsigned int port);
	bool getHandle();
	bool disconn();
	
	bool exec(const char* cmd);
	
//	RedisVar getInt(const char* table, unsigned long long id);
//	RedisVar getInt(const char* table, const char* id);
//	bool setInt(const char* table, unsigned long long id, unsigned long long value, unsigned int secs = 0);
//	bool setInt(const char* table, const char* id, unsigned long long value, unsigned int secs = 0);
//
//	RedisVar incrInt(const char* table, unsigned long long id);
//	RedisVar incrInt(const char* table, const char* id);
//	RedisVar decrInt(const char* table, unsigned long long id);
//	RedisVar decrInt(const char* table, const char* id);
//	
//	RedisVar getString(const char* table, unsigned long long id);
//	RedisVar getString(const char* table, const char* id);
//	bool setString(const char* table, unsigned long long id, const char* value);
//	bool setString(const char* table, const char* id, const char* value);
//
//	int getBin(const char* table, unsigned long long id, RedisVar& var);
//	int getBin(const char* table, const char* id, RedisVar& var);
//	int getBin(const char* table, unsigned long long id, char* buff, int len);
//	int getBin(const char* table, const char* id, char* buff, int len);
//	bool setBin(const char* table, unsigned long long id, const char* value, size_t len, unsigned int secs = 0);
//	bool setBin(const char* table, const char* id, const char* value, size_t len, unsigned int secs = 0);
//	bool setBinStr(const char* table, unsigned long long id, const string& str, unsigned int secs = 0) { return setBin(table, id, str.c_str(), str.size(), secs); };
//	bool setBinStr(const char* table, const char* id, const string& str, unsigned int secs = 0) { return setBin(table, id, str.c_str(), str.size(), secs); };
//	
//	// set
//	bool setSet(const char* table, unsigned long long id, unsigned long long value);
//	bool setSet(const char* table, unsigned long long id, const char* value);
//	bool setSet(const char* table, const char* id, unsigned long long value);
//	bool setSet(const char* table, const char* id, const char* value);
//	bool getSet(const char* table, unsigned long long id, std::set<std::string>& valueset);
//	bool getSet(const char* table, unsigned long long id, std::set<unsigned long long>& valueset);
//	bool getSet(const char* table, const char* id, std::set<std::string>& valueset);
//	bool getSet(const char* table, const char* id, std::set<unsigned long long>& valueset);
//
//	// 查找指定set中的某值是否存在
//	bool checkSet(const char* table, unsigned long long id, const unsigned long long value);
//	bool checkSet(const char* table, unsigned long long id, const char* value);
//	bool checkSet(const char* table, const char* id, const unsigned long long value);
//	bool checkSet(const char* table, const char* id, const char* value);
//
//	// 删除指定set中指定值
//	bool delSet(const char* table, unsigned long long id, const unsigned long long value);
//	bool delSet(const char* table, const char* id, const unsigned long long value);
//	bool delSet(const char* table, unsigned long long id, const char* value);
//	bool delSet(const char* table, const char* id, const char* value);
//
//	// sorted set
//	bool setZSet(const char* table, const char* id, unsigned long long score, const char* member);
//	bool delZSet(const char* table, const char* id, const char *member);
//	bool setZSet(const char* table, const char* id, unsigned long long score, unsigned long long member);
//	
//	bool delZSet(const char* table, const char* id, unsigned long long member);
//	bool delZSetByRank(const char* table, const char* id, int start, int end);
//	bool delZSetByScore(const char* table, const char* id, long long start, long long end);
//	RedisVar addZSetScore(const char* table, const char* id, const char *member, const long long score = 1);
//	RedisVar getZScore(const char* table, const char* id, const char* member);
//	RedisVar getZScore(const char* table, const char* id, unsigned long long member) { std::ostringstream smember; smember << member; return getZScore(table, id, smember.str().c_str()); };
//	RedisVar getZRank(const char* table, const char* id, const char* member, bool reverse = false);
//	bool getZSetByRank(const char* table, const char* id, std::vector<std::string>& valueset, int start, int end, bool reverse = false);
//	bool getZSetByRank(const char* table, const char* id, std::set<std::string>& valueset, int start, int end, bool reverse = false);
//	bool getZSetByRankWithScore(const char* table, const char* id, std::map<std::string, unsigned long long>& valuemap, int start, int end, bool reverse = false);
//	bool getZSetByScore(const char* table, const char* id, std::vector<std::string>& valuevec, long long start, long long end, int offset = 0, int count = 64, bool reverse = false);
//	bool getZSetByScoreWithScore(const char* table, const char* id, std::map<std::string, unsigned long long>& valuemap, long long start, long long end, int offset = 0, int count = 64, bool reverse = false);
//	unsigned int getZSetCount(const char* table, const char* id, unsigned long long start = 0, unsigned long long end = -1);
//
//	bool setZSet(const char* table, unsigned long long id, unsigned long long score, const char* member) { std::ostringstream sid; sid << id; return setZSet(table, sid.str().c_str(), score, member); };
//	bool delZSet(const char* table, unsigned long long id, const char *member) { std::ostringstream sid; sid << id; return delZSet(table, sid.str().c_str(), member); };
//	bool setZSet(const char* table, unsigned long long id, unsigned long long score, const unsigned long long member) { std::ostringstream sid; sid << id; return setZSet(table, sid.str().c_str(), score, member); };
//	
//	bool delZSet(const char* table, unsigned long long id, unsigned long long member) { std::ostringstream sid; sid << id; return delZSet(table, sid.str().c_str(), member); };
//	bool delZSetByRank(const char* table, unsigned long long id, int start, int end) { std::ostringstream sid; sid << id; return delZSetByRank(table, sid.str().c_str(), start, end); };
//	bool delZSetByScore(const char* table, unsigned long long id, long long start, long long end) { std::ostringstream sid; sid << id; return delZSetByScore(table, sid.str().c_str(), start, end); };
//	RedisVar addZSetScore(const char* table, unsigned long long id, const char *member, const long long score = 1) { std::ostringstream sid; sid << id; return addZSetScore(table, sid.str().c_str(), member, score); };
//	RedisVar addZSetScore(const char* table, const char* id, unsigned long long member, const long long score = 1) { std::ostringstream sid; sid << member; return addZSetScore(table, id, sid.str().c_str(), score); };
//	RedisVar addZSetScore(const char* table, unsigned long long id, unsigned long long member, const long long score = 1) { std::ostringstream sid; sid << id; return addZSetScore(table, sid.str().c_str(), member, score); };
//	RedisVar getZScore(const char* table, unsigned long long id, const char* member) { std::ostringstream sid; sid << id; return getZScore(table, sid.str().c_str(), member); };
//	RedisVar getZScore(const char* table, unsigned long long id, unsigned long long member) { std::ostringstream sid; sid << id; std::ostringstream mem; mem << member; return getZScore(table, sid.str().c_str(), mem.str().c_str()); };
//	RedisVar getZRank(const char* table, unsigned long long id, const char* member, bool reverse = false) { std::ostringstream sid; sid << id; return getZRank(table, sid.str().c_str(), member, reverse); };
//	RedisVar getZRank(const char* table, unsigned long long id, unsigned long long member, bool reverse = false) { std::ostringstream sid; sid << id; std::ostringstream sm; sm << member; return getZRank(table, sid.str().c_str(), sm.str().c_str(), reverse); };
//	bool getZSetByRank(const char* table, unsigned long long id, std::vector<std::string>& valueset, int start, int end, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetByRank(table, sid.str().c_str(), valueset, start, end, reverse); };
//	bool getZSetByRank(const char* table, unsigned long long id, std::set<std::string>& valueset, int start, int end, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetByRank(table, sid.str().c_str(), valueset, start, end, reverse); };
//	bool getZSetByRankWithScore(const char* table, unsigned long long id, std::map<std::string, unsigned long long>& valuemap, int start, int end, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetByRankWithScore(table, sid.str().c_str(), valuemap, start, end, reverse); };
//	bool getZSetByScore(const char* table, unsigned long long id, std::vector<std::string>& valuevec, long long start, long long end, int offset = 0, int count = 64, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetByScore(table, sid.str().c_str(), valuevec, start, end, offset, count, reverse); };
//	bool getZSetByScoreWithScore(const char* table, unsigned long long id, std::map<std::string, unsigned long long>& valuemap, long long start, long long end, int offset = 0, int count = 64, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetByScoreWithScore(table, sid.str().c_str(), valuemap, start, end, offset, count, reverse); };
//	unsigned int getZSetCount(const char* table, unsigned long long id, unsigned long long start = 0, unsigned long long end = -1) { std::ostringstream sid; sid << id; return getZSetCount(table, sid.str().c_str(), start, end); };
//
//	bool setZSetBin(const char* table, const char* id, const unsigned long long score, const char* member, size_t len);
//	bool setZSetBin(const char* table, unsigned long long id, const unsigned long long score, const char* member, size_t len) { std::ostringstream sid; sid << id; return setZSetBin(table, sid.str().c_str(), score, member, len); }
//	bool getZSetBinByScoreWithScore(const char* table, const char* id, std::vector<RedisVar>& ret, long long start, long long end, int offset = 0, int count = 64, bool reverse = false);
//	bool getZSetBinByScoreWithScore(const char* table, unsigned long long id, std::vector<RedisVar>& ret, long long start, long long end, int offset = 0, int count = 64, bool reverse = false) { std::ostringstream sid; sid << id; return getZSetBinByScoreWithScore(table, sid.str().c_str(), ret, start, end, offset, count, reverse); };
//	// mset
//	unsigned int mset(const char* pattern, ...);
//private:
//	unsigned int mget(std::vector<RedisVar>& ret, const char* pattern, ...);
//public:
//	unsigned int mget(const char* table, std::vector<RedisVar>& ret, std::vector<unsigned long long>& ids);
//	unsigned int mget(const char* table, std::vector<RedisVar>& ret, std::vector<std::string>& ids);
//	unsigned int mget(const char* table, std::map<unsigned long long, RedisVar>& valuemap);
//	unsigned int mget(const char* table, std::map<std::string, RedisVar>& valuemap);
//
//	// hmset
//	unsigned int hmset(const char* pattern, ...);
//	unsigned int hmset(const char* szTable, const char* szId, const char* pattern, ...);
//	unsigned int hmset(const char* szTable, const unsigned long long Id, const char* pattern, ...);
//	unsigned int hmset(const char* szTable, const char* pattern, ...);
//	unsigned int hmget(std::vector<RedisVar>& ret, const char* pattern, ...);
//	unsigned int hmget(const char* szTable, const char* szId, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret);
//	unsigned int hmget(const char* szTable, const char* szId, const char* strColumn, std::vector<RedisVar>& ret);
//	unsigned int hmget(const char* szTable, unsigned long long dwpId, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret);
//	unsigned int hmget(const char* szTable, std::vector<std::string>& vecstrColumn, std::vector<RedisVar>& ret);
//	unsigned int hmget(const char* szTable, unsigned long long dwpId, const char* strColumn, std::vector<RedisVar>& ret);
//	unsigned int hmget(const char* szTable, const char* strColumn, std::vector<RedisVar>& ret);
//	unsigned int hset(const char* szTable, const char* szField, const char* value);
//	unsigned int hset(const char* szTable, const char* Id,const char* szField, const char* value)
//	{
//		std::ostringstream sid; sid << szTable << "_" <<Id;
//		return hset(sid.str().c_str(), szField, value);
//	}
//	unsigned int hset(const char* szTable, unsigned long long Id, const char* szField, const char* value)
//	{
//		std::ostringstream sid; sid << szTable << "_" << Id;
//		return hset(sid.str().c_str(), szField, value);
//	}
//	bool hdel(const char* szTable, unsigned long long Id, const char* szFields) { std::ostringstream sid; sid << Id; return hdel(szTable, sid.str().c_str(), szFields); };
//	bool hdel(const char* szTable, const char* Id, const char* szFields);
//	//unsigned int hmgetAll(std::vector<RedisVar>& ret, const char* table, ...);
//
//	RedisVar hgetInt(const char* table, const char* id, const char* field);
//	RedisVar hgetString(const char* table, const char* id, const char* field);
//	RedisVar hgetBin(const char* table, const char* id, const char* field);
//
//	RedisVar hgetInt(const char* table, unsigned long long id, const char* field)
//	{
//		std::ostringstream sid; sid << id; return hgetInt(table, sid.str().c_str(), field);
//	}
//	RedisVar hgetString(const char* table, unsigned long long id, const char* field)
//	{
//		std::ostringstream sid; sid << id; return hgetString(table, sid.str().c_str(), field);
//	}
//	RedisVar hgetBin(const char* table, unsigned long long id, const char* field)
//	{
//		std::ostringstream sid; sid << id; return hgetBin(table, sid.str().c_str(), field);
//	}
//
//	bool del(const char* table, unsigned long long id);
//	bool del(const char* table, const char* id);
//	bool exist(const char* table, unsigned long long id);
//	bool exist(const char* table, const char* id);
//
//	// 事务
//	bool watch(const char* table, unsigned long long id);
//	bool watch(const char* table, const char* id);
//	bool unwatch();
//	bool lock();		// multi
//	bool unlock();		// exec
//	bool islockedtable(const char* table, const char* id);
//	bool locktable(const char* table, const char* id, int block, unsigned int secs);
//	bool locktable(const char* table, const char* id, int loop, int block, unsigned int secs);
//	bool locktable(const char* table, unsigned long long id, int block, unsigned int secs) { std::ostringstream sid; sid << id; return locktable(table, sid.str().c_str(), block, secs); };
//	bool locktable(const char* table, unsigned long long id, int loop, int block, unsigned int secs) { std::ostringstream sid; sid << id; return locktable(table, sid.str().c_str(), loop, block, secs); };
//	bool unlocktable(const char* table, const char* id, bool del = true);
//	bool unlocktable(const char* table, unsigned long long id, bool del = true) { std::ostringstream sid; sid << id; return unlocktable(table, sid.str().c_str(), del); };
//	bool dellocktable(const char* table, const char* id);
//	bool dellocktable(const char* table, unsigned long long id) { std::ostringstream sid; sid << id; return dellocktable(table, sid.str().c_str()); };
//
//	RedisVar getValue(int type);
//
//	bool getKeys(const char* key, std::vector<std::string> &vec_keys);
private:
	redisContext* c;
	redisReply* reply;
	std::string host;
	unsigned int port;
	unsigned long long _key;
	bool redis_cluster;
public:
	static redisconn* Instance();
};

struct stRedisConfig
{
	std::string m_sIP;
	unsigned short m_uiPort;
	unsigned short m_uiThreadNum;
	bool m_bFlush;
	bool m_bCluster;
	std::string url() {
		char buff[100] = { 0 };
		sprintf(buff, "%s@%s:%d", "redis", m_sIP.c_str(), m_uiPort);
		return buff;
	};
	static unsigned long long key(std::string szIP, unsigned short uiPort) {
		unsigned int a = 0, b = 0, c = 0, d = 0;
		if (sscanf(szIP.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
			return 0;

		union unAddress
		{
			unsigned int uiIPAddress;
			unsigned char uiIPAddresss[4];
		} uAddress;
		uAddress.uiIPAddresss[0] = (unsigned char)a;
		uAddress.uiIPAddresss[1] = (unsigned char)b;
		uAddress.uiIPAddresss[2] = (unsigned char)c;
		uAddress.uiIPAddresss[3] = (unsigned char)d;

		unsigned long long key = uAddress.uiIPAddress;
		return key << 32 | uiPort;
	};
};

// -----------------------------------------------------------------------------------------
typedef std::vector<stRedisConfig> REDISDBURL;
typedef std::map<unsigned long long, stRedisConfig> REDISDBURLMAP;
typedef std::vector<redisconn*> REDISDBVEC;
typedef std::map<unsigned long long, REDISDBVEC > REDISDBMAP;
typedef std::map<unsigned long, redisconn*> THREADREDISDB;
class redispool
{
public:
	redispool(){}
	~redispool();
	void init(REDISDBURL& cfgs);
	void exit();
	void flush();
	void save(bool bg = false) {
		if (bg)
			conn(0)->exec("BGSAVE");
		else
			conn(0)->exec("SAVE");
	};
	redisconn* conn(unsigned long i = 0)
	{
		//if (i == 0) i = JyThreadId();
		THREADREDISDB::iterator it = m_threadconn.find(i);
		if (it != m_threadconn.end())
			return it->second;
		redisconn* conn = NULL;
		m_lock.Lock();
		conn = __create();
		m_threadconn[i] = conn;
		m_lock.UnLock();
		return conn;
	};
	static redispool* Instance();
private:
	redisconn* __create();
	REDISDBURLMAP m_dburls;
	REDISDBMAP m_dbpool;
	REDISDBVEC m_dbs;
	THREADREDISDB m_threadconn;
	FxCriticalLock m_lock;
};
// -----------------------------------------------------------------------------------------
#endif
