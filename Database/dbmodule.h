#ifndef __DBDBMODULE_H_2009_0824__
#define __DBDBMODULE_H_2009_0824__

#include <map>
#include "fxdb.h"
#include <fxmeta.h>
#include "singleton.h"
#include "dbclient.h"
#include "dynamicpoolex.h"
#include "lock.h"

class FxDBModule : public IDBModule
{
	FxDBModule(void);
	virtual ~FxDBModule();
    friend class FxMySqlClient;

	DECLARE_SINGLETON(FxDBModule)
public:
	virtual void        Release(void);
	virtual const char* GetModuleName(void);

	virtual bool        Open(SDBConnInfo& DBInfo);
	virtual void        Close(UINT32 dwDBId);
	virtual bool        AddQuery(IQuery *poQuery);
	virtual bool        Run(UINT32 dwCount);

    bool                Init();
    void                Uninit();
    void                AddResult(IQuery* poQuery);

    FxMySQLReader*      FetchReader();
    void                ReleaseReader(FxMySQLReader* poReader);
    FxMySqlClient *     FindDBClient(UINT32 dwDBId);

private:
    bool                __CallBackResult();
	void                __ClearResult();

private:
	FxCriticalLock		    m_oLock;
    TDynamicPoolEx<FxMySQLReader>   m_oReaderPool;
	std::list<IQuery*>       m_oResultList;
	std::map<UINT32, FxMySqlClient*> m_mapDBClient;
};

#endif  // __DBDBMODULE_H_2009_0824__

