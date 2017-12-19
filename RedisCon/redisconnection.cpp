#include "redisconnection.h"
#include "strhelper.h"

FxRedisConnection::FxRedisConnection(void)
	: m_pRedisContext(NULL)
	, m_dwPort(0)
{
	//m_myConnection = NULL;
}

FxRedisConnection::~FxRedisConnection(void)
{
	//if (m_myConnection != NULL)
	//{
	//	mysql_close(m_myConnection);
	//	   m_myConnection = NULL;
	//}
}

bool FxRedisConnection::Connect(const std::string& szHost, unsigned int dwPort)
{
	m_szHost = szHost;
	m_dwPort = dwPort;
	return ReConnect();
}

bool FxRedisConnection::Close()
{
	if (m_pRedisContext)
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = NULL;
	}
	return true;
}

bool FxRedisConnection::ReConnect()
{
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	Close();

	this->m_pRedisContext = redisConnectWithTimeout(m_szHost.c_str(), m_dwPort, timeout);
	if (m_pRedisContext->err)
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = NULL;
		return false;
	}

	return true;
}

INT32 FxRedisConnection::Query(const char* pszCMD)
{
	redisReply* preply = NULL;
	if (CheckConnection())
	{
		preply = (redisReply*)redisCommand(m_pRedisContext, pszCMD);
	}
	if (preply == NULL)
	{
		this->Close();
		return -1;
	}
	if (preply->type == REDIS_REPLY_ERROR)
	{
		freeReplyObject(preply);
		preply = NULL;
		return REDIS_REPLY_ERROR;
	}
	freeReplyObject(preply);
	preply = NULL;
	return 0;
}

INT32 FxRedisConnection::Query(const char* pszCMD, FxRedisReader& reader)
{
	if(false == CheckConnection())
	{
		return -1;
	}

	reader.reply = (redisReply*)redisCommand(m_pRedisContext, pszCMD);

	if (reader.reply == NULL)
	{
		this->Close();
		return -1;
	}

	if (reader.reply->type == REDIS_REPLY_ERROR)
	{
		freeReplyObject(reader.reply);
		reader.reply = NULL;
		return REDIS_REPLY_ERROR;
	}

	return 0;
}

bool FxRedisConnection::CheckConnection()
{
	if (!m_pRedisContext)
	{
		if (!m_pRedisContext)
		{
			ReConnect();
		}
		//LOG_INFO("[redisconn] reconnect redis:%s,%d", this->host.c_str(), this->port);
	}
	if (!m_pRedisContext)
	{
		//LOG_ERROR("[redisconn] reconnect fail, redis:%s,%d", this->host.c_str(), this->port);
		return false;
	}
	return true;
}

