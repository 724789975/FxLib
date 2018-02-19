#ifndef __PlayerSession_H__
#define __PlayerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"
#include "gamedefine.h"
#include "lock.h"

class CPlayerSession : public FxSession
{
public:
	CPlayerSession();
	virtual ~CPlayerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	void				OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen);
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CWebSocketPlayerSession : public CPlayerSession
{
public:
	CWebSocketPlayerSession();
	~CWebSocketPlayerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
	virtual void Release(void);
private:
	WebSocketDataHeader m_oWebSocketDataHeader;
};

class WebSocketPlayerSessionManager : public IFxSessionFactory
{
public:
	WebSocketPlayerSessionManager();
	~WebSocketPlayerSessionManager();

	virtual FxSession* CreateSession();

	virtual void Release(FxSession* pSession);
	virtual void Release(CWebSocketPlayerSession* pSession);

private:
	CWebSocketPlayerSession m_oWebSocketPlayerSessions[MAXCLIENTNUM];
	FxCriticalLock m_oLock;
};

#endif	//__PlayerSession_H__
