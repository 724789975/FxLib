#ifndef __CPSOCKMGR_H__
#define __CPSOCKMGR_H__

#include <list>
#include <map>
#include "mysock.h"
#include "dynamicpoolex.h"
#include "ifnet.h"

class FxMySockMgr
{
	FxMySockMgr();
	~FxMySockMgr();

	DECLARE_SINGLETON(FxMySockMgr)

public:
	bool							Init(INT32 nMax);
	void							Uninit();

	FxConnectSock *					Create();
	FxListenSock*					Create(UINT32 dwListenId, IFxSessionFactory* pSessionFactory);
	void							Release(FxConnectSock* poSock);
	void							Release(UINT32 dwListenId);

protected:
	INT32							m_nSockCount;
	time_t							m_nLastCheckTime;
	TDynamicPoolEx<FxConnectSock>	m_oCPSockPool;
	UINT32							m_dwNextId;

	std::map<UINT32, FxListenSock>	m_mapListenSocks;
};

#endif	// __CPSOCKMGR_H__
