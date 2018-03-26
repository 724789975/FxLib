#ifndef __CSlaveServerSession_H__
#define __CSlaveServerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"
#include "gamedefine.h"
#include "lock.h"

class CSlaveServerSession : public FxSession
{
public:
	CSlaveServerSession();
	~CSlaveServerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	//void				OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen);
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CWebSocketSlaveServerSession : public CSlaveServerSession
{
public:
	CWebSocketSlaveServerSession();
	~CWebSocketSlaveServerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
	virtual void Release(void);
private:
	WebSocketDataHeader m_oWebSocketDataHeader;
};

class WebSocketSlaveServerSessionManager : public IFxSessionFactory
{
public:
	WebSocketSlaveServerSessionManager();
	~WebSocketSlaveServerSessionManager();

	virtual FxSession* CreateSession();

	virtual void Release(FxSession* pSession);
	virtual void Release(CWebSocketSlaveServerSession* pSession);

private:
	TDynamicPoolEx<CWebSocketSlaveServerSession> m_poolSessions;
	CWebSocketSlaveServerSession m_oWebSocketSlaveServerSessions[MAXSLAVESERVERNUM];
	FxCriticalLock m_oLock;
};



#endif // !__CSlaveServerSession_H__
