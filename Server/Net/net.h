#ifndef __NET_H_2009_0825__
#define __NET_H_2009_0825__

#include "ifnet.h"
#include "mytcpsock.h"
#include "dynamicpoolex.h"
#include <map>
#include <vector>

struct SSockNetEvent
{
	IFxSocket* poSock;
	SNetEvent oEvent;
};

class FxIoThread;

class FxNetModule : public TSingleton<FxNetModule>, public IFxNet
{
public:
	FxNetModule();
	virtual ~FxNetModule();
	//DECLARE_SINGLETON(FxNetModule);

	virtual bool				Init();
	virtual bool				Run(UINT32 dwCount = 0xffffffff);
	virtual void				Release();
	virtual bool				SetNetOpt(ENetOpt eOpt, int nValue);

	virtual IFxListenSocket*	Listen(IFxSessionFactory* pSessionFactory, ESocketType eSocketListenType, UINT32 dwIP, UINT16& dwPort);

	virtual SOCKET				TcpConnect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect = false);

	virtual SOCKET				UdpConnect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect = false);

	void						Uninit();

	bool						PushNetEvent(IFxSocket* poSock, SNetEvent oEvent);
	FxIoThread*					FetchIoThread(UINT32 dwSockId);

	int							GetThreadCount() { return m_nNetThreadCount; }

private:
	bool						__CreateComponent();
	bool						__InitComponent();
	void						__UninitComponent();
	void						__DestroyComponent();

private:
	TEventQueue<SSockNetEvent>*	m_pEventQueue;
	int							m_nNetThreadCount;
	FxIoThread*					m_pEpollHandlers;
	int							m_nMaxConnectionCount;
	int							m_nTotalEventCount;
	bool						m_bInit;
};

#endif	// __NET_H_2009_0825__

