#ifndef __NET_H_2009_0825__
#define __NET_H_2009_0825__

#include "ifnet.h"
#include "mysock.h"
#include "dynamicpoolex.h"
#include <map>
#include <vector>

class FxIoThread;

class FxNetModule : public IFxNet
{
public:
	FxNetModule();
	virtual ~FxNetModule();
	DECLARE_SINGLETON(FxNetModule);

	virtual bool	Init();
	virtual bool	Run(UINT32 dwCount = 0xffffffff);
	virtual void	Release();
	virtual bool	SetNetOpt(ENetOpt eOpt, int nValue);

	virtual SOCKET			Connect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect = false);
	virtual IFxListenSocket* Listen(IFxSessionFactory* pSessionFactory, UINT32 dwListenId, UINT32 dwIP, UINT16 dwPort);

	void				Uninit();

	bool			PushNetEvent(IFxSocket* poSock);
	FxIoThread*		FetchIoThread(UINT32 dwSockId);

private:
	bool					__CreateComponent();
	bool					__InitComponent();
	void					__UninitComponent();
	void					__DestroyComponent();

private:
	TEventQueue<IFxSocket*>*	m_pEventQueue;
	int							m_nNetThreadCount;
	FxIoThread*					m_pEpollHandlers;
	int							m_nMaxConnectionCount;
	int							m_nTotalEventCount;
	bool						m_bInit;
};

#endif	// __NET_H_2009_0825__

