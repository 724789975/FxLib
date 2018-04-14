#ifndef __REDIS_MODULE_H__
#define __REDIS_MODULE_H__

#include <map>
#include "fxredis.h"
#include <fxmeta.h>
#include "singleton.h"
#include "redisclient.h"
#include "dynamicpoolex.h"
#include "lock.h"
#include "redisquery.h"

class FxRedisModule : public TSingleton<FxRedisModule>, public IRedisModule
{
public:
	FxRedisModule(void);
	virtual ~FxRedisModule();
    friend class FxRedisClient;

	//DECLARE_SINGLETON(FxDBModule)
	virtual void        Release(void);
	virtual const char* GetModuleName(void);

	virtual bool		Open(const char* szHost, unsigned int dwPort, std::string szPassword, unsigned int dwRedisId);
	virtual void        Close(UINT32 dwDBId);
	virtual bool		AddQuery(IRedisQuery*poQuery);
	virtual bool        Run(UINT32 dwCount);

	//主线程阻塞查询
	virtual void		QueryDirect(IRedisQuery* poQuery);

    bool                Init();
    void                Uninit();
    void				AddResult(IRedisQuery* poQuery);

    FxRedisReader*      FetchReader();
    void                ReleaseReader(FxRedisReader* poReader);
    FxRedisClient *     FindDBClient(UINT32 dwDBId);

private:
    bool                __CallBackResult();
	void                __ClearResult();

private:
	FxCriticalLock		    m_oLock;
    TDynamicPoolEx<FxRedisReader>   m_oReaderPool;
	std::list<IRedisQuery*>       m_oResultList;
	std::map<UINT32, FxRedisClient*> m_mapDBClient;
	std::map<UINT32, FxRedisQuery> m_mapRedisQuery;
};

#endif  // __REDIS_MODULE_H__

