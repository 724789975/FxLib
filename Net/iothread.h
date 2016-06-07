#ifndef __IOThread_H__
#define __IOThread_H__

#include "mytcpsock.h"
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

	void					SetStoped() { m_bStop = true; }

	bool					Start();

	bool					Init(UINT32 dwMaxSock);
	void					Uninit();

#ifdef WIN32
	bool					AddEvent(int hSock, IFxSocket* poSock);
#else
	bool					AddEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock);
	bool					ChangeEvent(int hSock, UINT32 dwEvents, IFxSocket* poSock);
	bool					DelEvent(int hSock);
#endif // WIN32

	// win下为完成端口 linux下为epoll
#ifdef WIN32
	HANDLE					GetHandle();
#else
	int						GetHandle();
	int						WaitEvents(int nMilliSecond);
	epoll_event*			GetEvent(int nIndex);
#endif // WIN32
		


private:
	void					 __DealEpollSock();
	bool					 __DealEpollData();

protected:
	IFxThreadHandler*		m_poThrdHandler;

	UINT32					m_dwMaxSock;
	bool					m_bStop;

#ifdef WIN32
#else
	int					m_hEpoll;
	epoll_event*		m_pEvents;
#endif // WIN32

};

#endif // __IOThread_H__
