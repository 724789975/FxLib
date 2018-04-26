#ifndef __ServerSession_H__
#define __ServerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"

class CServerSession : public FxSession
{
public:
	CServerSession();
	virtual ~CServerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

private:
	char m_dataRecvBuf[1024 * 1024];

	CallBackDispatcher::ProtoCallBackDispatch<CServerSession, CServerSession> m_oProtoDispatch;
};

class CWebSocketServerSession : public CServerSession
{
public:
	CWebSocketServerSession();
	~CWebSocketServerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
	virtual void Release(void);
private:
	WebSocketDataHeader m_oWebSocketDataHeader;
};

class WebSocketServerSessionManager : public IFxSessionFactory
{
public:
	WebSocketServerSessionManager();
	~WebSocketServerSessionManager();

	virtual CWebSocketServerSession* CreateSession();

	virtual void Release(FxSession* pSession);

private:
	CWebSocketServerSession* m_pCreated;
	CWebSocketServerSession m_oWebSocketSlaveServerSession;
};

#endif	//__ServerSession_H__
