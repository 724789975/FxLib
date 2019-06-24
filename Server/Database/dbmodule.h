#ifndef __DBDBMODULE_H_2009_0824__
#define __DBDBMODULE_H_2009_0824__

#include <map>
#include "fxdb.h"
#include <fxmeta.h>
#include "singleton.h"
#include "dbclient.h"
#include "dynamicpoolex.h"
#include "lock.h"

class FxDBModule : public TSingleton<FxDBModule>, public IDBModule
{
public:
	FxDBModule(void);
	virtual ~FxDBModule();
    friend class FxMySqlClient;

	//DECLARE_SINGLETON(FxDBModule)
	virtual void        Release(void);
	virtual const char* GetModuleName(void);

	virtual bool        Open(SDBConnInfo& DBInfo);
	virtual void        Close(unsigned int dwDBId);
	virtual bool        AddQuery(IQuery *poQuery);
	virtual bool        Run(unsigned int dwCount);

    bool                Init();
    void                Uninit();
    void                AddResult(IQuery* poQuery);

    FxMySQLReader*      FetchReader();
    void                ReleaseReader(FxMySQLReader* poReader);
    FxMySqlClient *     FindDBClient(unsigned int dwDBId);

private:
    bool                __CallBackResult();
	void                __ClearResult();

private:
	FxCriticalLock		    m_oLock;
    TDynamicPoolEx<FxMySQLReader>   m_oReaderPool;
	std::list<IQuery*>       m_oResultList;
	std::map<unsigned int, FxMySqlClient*> m_mapDBClient;
};

#endif  // __DBDBMODULE_H_2009_0824__

