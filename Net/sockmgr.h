#ifndef __CPSOCKMGR_H__
#define __CPSOCKMGR_H__

#include <list>
#include <map>
#include "mytcpsock.h"
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

	FxTCPConnectSock *					Create();
	FxTCPListenSock*					Create(UINT32 dwListenId, IFxSessionFactory* pSessionFactory);
	void							Release(FxTCPConnectSock* poSock);
	void							Release(UINT32 dwListenId);

protected:
	INT32							m_nSockCount;
	time_t							m_nLastCheckTime;
	TDynamicPoolEx<FxTCPConnectSock>	m_oCPSockPool;
	UINT32							m_dwNextId;

	std::map<UINT32, FxTCPListenSock>	m_mapListenSocks;
};

#endif	// __CPSOCKMGR_H__
