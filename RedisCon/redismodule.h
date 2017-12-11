#ifndef __DBDBMODULE_H_2009_0824__
#define __DBDBMODULE_H_2009_0824__

#include <map>
#include "fxredis.h"
#include <fxmeta.h>
#include "singleton.h"
#include "redisclient.h"
#include "dynamicpoolex.h"
#include "lock.h"

class FxRedisModule : public TSingleton<FxRedisModule>, public IRedisModule
{
public:
	FxRedisModule(void);
	virtual ~FxRedisModule();
    friend class FxRedisClient;

	//DECLARE_SINGLETON(FxDBModule)
	virtual void        Release(void);
	virtual const char* GetModuleName(void);

	virtual bool		Open(const char* szHost, unsigned int dwPort, unsigned int dwRedisId);
	virtual void        Close(UINT32 dwDBId);
	virtual bool AddQuery(IRedisQuery*poQuery);
	virtual bool        Run(UINT32 dwCount);

    bool                Init();
    void                Uninit();
    void AddResult(IRedisQuery* poQuery);

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
};

#endif  // __DBDBMODULE_H_2009_0824__

