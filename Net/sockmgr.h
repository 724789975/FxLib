#ifndef __CPSOCKMGR_H__
#define __CPSOCKMGR_H__

#include <list>
#include <map>
#include "mytcpsock.h"
#include "myudpsock.h"
#include "dynamicpoolex.h"
#include "ifnet.h"

class FxMySockMgr
{
	FxMySockMgr();
	~FxMySockMgr();

	DECLARE_SINGLETON(FxMySockMgr)

public:
	bool								Init(INT32 nMax);
	void								Uninit();

	FxTCPConnectSock *					CreateTcpSock();
	FxTCPListenSock*					CreateTcpSock(UINT32 dwListenId, IFxSessionFactory* pSessionFactory);
	void								ReleaseTcpSock(FxTCPConnectSock* poSock);
	void								ReleaseTcpSock(UINT32 dwListenId);

	FxUDPConnectSock *					CreateUdpSock();
	FxUDPListenSock*					CreateUdpSock(UINT32 dwListenId, IFxSessionFactory* pSessionFactory);
	void								ReleaseUdpSock(FxUDPConnectSock* poSock);
	void								ReleaseUdpSock(UINT32 dwListenId);

protected:
	UINT32								m_dwNextId;

	TDynamicPoolEx<FxTCPConnectSock>	m_oTCPSockPool;
	std::map<UINT32, FxTCPListenSock>	m_mapTcpListenSocks;

	TDynamicPoolEx<FxUDPConnectSock>	m_oUDPSockPool;
	std::map<UINT32, FxUDPListenSock>	m_mapUdpListenSocks;
};

#endif	// __CPSOCKMGR_H__
