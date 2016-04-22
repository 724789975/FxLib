#ifndef __IOThread_H__
#define __IOThread_H__

#include "mysock.h"
#include <vector>
#ifdef WIN32
#else
#include <sys/epoll.h>
#endif // WIN32


class FxIoThread : public IFxThread
{
public:
	FxIoThread();
	virtual ~FxIoThread();

	virtual void			ThrdFunc();
	virtual void			Stop();

	bool					Start();

	bool					Init(UINT32 dwMaxSock);
	void					Uninit();

	bool PushSock(IFxSocket* poSock);
#ifdef WIN32
	bool AddEvent(int hSock, IFxSocket* poSock);
#else
	bool AddEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock);
	bool ChangeEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock);
	bool DelEvent(int hSock);
#endif // WIN32

	// win下为完成端口 linux下为epoll
#ifdef WIN32
	HANDLE GetHandle();
#else
	int GetHandle();
	int						WaitEvents(int nMilliSecond);
	epoll_event*			GetEvent(int nIndex);
#endif // WIN32
		


private:
	void					 __DealEpollSock();
	bool					 __DealEpollData();

protected:
	IFxThreadHandler*	m_poThrdHandler;

	UINT32				m_dwMaxSock;
	bool				m_bStop;

	TEventQueue<IFxSocket*>       m_sockQueue;		//因为事件都是在主线程处理的 所以 先把事件暂存一下 等io线程处理好后 再投递到主线程//
#ifdef WIN32
#else
	int					m_hEpoll;
	epoll_event*		m_pEvents;
#endif // WIN32

};

#endif // __IOThread_H__
