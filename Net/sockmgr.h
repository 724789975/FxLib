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

	FxTCPConnectSock *					CreateCommonTcp();
	FxWebSocketConnect*					CreateWebSocket();
	FxTCPListenSock*					CreateCommonTcpListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	FxWebSocketListen*					CreateWebSocketListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	void								Release(FxTCPConnectSock* poSock);
	void								Release(FxWebSocketConnect* poSock);

	FxUDPConnectSock *					CreateUdpSock();
	FxUDPListenSock*					CreateUdpSockListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	void								Release(FxUDPConnectSock* poSock);

protected:
	UINT32								m_dwNextId;

	TDynamicPoolEx<FxTCPConnectSock>	m_oTCPSockPool;
	TDynamicPoolEx<FxWebSocketConnect>	m_oWebSockPool;

	TDynamicPoolEx<FxUDPConnectSock>	m_oUDPSockPool;
	std::map<UINT32, IFxListenSocket*>	m_mapListenSocks;
};

#endif	// __CPSOCKMGR_H__
