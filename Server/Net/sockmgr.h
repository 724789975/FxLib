#ifndef __CPSOCKMGR_H__
#define __CPSOCKMGR_H__

#include <list>
#include <map>
#include "mytcpsock.h"
#include "myudpsock.h"
#include "dynamicpoolex.h"
#include "ifnet.h"
#include "singleton.h"

class FxMySockMgr : public TSingleton<FxMySockMgr>
{
public:
	FxMySockMgr();
	virtual ~FxMySockMgr();

	//DECLARE_SINGLETON(FxMySockMgr)

	bool								Init(INT32 nMax);
	void								Uninit();

	FxTCPConnectSock *					CreateCommonTcp();
	FxWebSocketConnect*					CreateWebSocket();
	FxHttpConnect*						CreateHttpConnect();
	FxTCPListenSock*					CreateCommonTcpListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	FxWebSocketListen*					CreateWebSocketListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	FxHttpListen*						CreateHttpListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	void								Release(FxTCPConnectSock* poSock);
	void								Release(FxWebSocketConnect* poSock);
	void								Release(FxHttpConnect* poSock);

	FxUDPConnectSock *					CreateUdpSock();
	FxUDPListenSock*					CreateUdpSockListen(UINT32 dwPort, IFxSessionFactory* pSessionFactory);
	void								Release(FxUDPConnectSock* poSock);

protected:
	UINT32								m_dwNextId;

	TDynamicPoolEx<FxTCPConnectSock>	m_oTCPSockPool;
	TDynamicPoolEx<FxWebSocketConnect>	m_oWebSockPool;
	TDynamicPoolEx<FxHttpConnect>	m_oHttpPool;

	TDynamicPoolEx<FxUDPConnectSock>	m_oUDPSockPool;
	std::map<UINT32, IFxListenSocket*>	m_mapListenSocks;
};

#endif	// __CPSOCKMGR_H__
