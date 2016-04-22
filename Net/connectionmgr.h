#ifndef __CONNECTIONMGR_H_NICK_2009_1203__
#define __CONNECTIONMGR_H_NICK_2009_1203__

#include <list>
#include "connection.h"
#include "dynamicpoolex.h"

typedef std::list<FxConnection*> TpListConnection;

class FxConnectionMgr
{
	FxConnectionMgr();
	~FxConnectionMgr();

	DECLARE_SINGLETON(FxConnectionMgr)

public:
	bool                    Init(INT32 nMax);
	void                    Uninit();

	FxConnection*           Create();
	void                    Release(FxConnection* poConnection);

protected:
	UINT32					m_dwNextId;
	INT32					m_nMaxConnection;
    TDynamicPoolEx<FxConnection> m_oConnPool;
};

#endif  // __CONNECTIONMGR_H_NICK_2009_1203__

