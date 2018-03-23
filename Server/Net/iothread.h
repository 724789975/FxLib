#ifndef __IOThread_H__
#define __IOThread_H__

#include "mytcpsock.h"
#include <vector>
#include <set>
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

	UINT32					GetThreadId();
	FILE*&					GetFile() { return m_pFile; }
	const char*				GetLogFile() { return m_szLogPath; }

	void					AddConnectSocket(IFxConnectSocket* pSock);
	void					DelConnectSocket(IFxConnectSocket* pSock);

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
	void					PushDelayCloseSock(IFxSocket* poSock);
#endif // WIN32

private:
	void					 __DealSock();
	bool					 __DealData();

protected:
	IFxThreadHandler*		m_poThrdHandler;

	UINT32					m_dwMaxSock;
	bool					m_bStop;

#ifdef WIN32
	HANDLE					m_hCompletionPort;
#else
	int						m_hEpoll;
	epoll_event*			m_pEvents;
	TEventQueue<IFxSocket*>	m_oDelayCloseSockQueue;
#endif // WIN32

	FILE*					m_pFile;
	char					m_szLogPath[64];

	//存放udp的连接指针 目前只在udp中用到 每0.01秒发送一次
	std::set<IFxConnectSocket*>	m_setConnectSockets;
	//最后一次更新的时间
	double					m_dLoatUpdateTime;
};

class FxHttpThread : public TSingleton<FxIoThread>
{
};


#endif // __IOThread_H__
