#ifndef __CONNECTIONMGR_H__
#define __CONNECTIONMGR_H__

#include <list>
#include "connection.h"
#include "dynamicpoolex.h"

typedef std::list<FxConnection*> TpListConnection;

class FxConnectionMgr : public TSingleton<FxConnectionMgr>
{
public:
	FxConnectionMgr();
	virtual ~FxConnectionMgr();

	//DECLARE_SINGLETON(FxConnectionMgr)

	bool                    Init(INT32 nMax);
	void                    Uninit();

	FxConnection*           Create();
	void                    Release(FxConnection* poConnection);

protected:
	UINT32					m_dwNextId;
	INT32					m_nMaxConnection;
    TDynamicPoolEx<FxConnection> m_oConnPool;
};

#endif  // __CONNECTIONMGR_H__

