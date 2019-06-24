#ifndef __CenterSession_H__
#define __CenterSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"

class CCenterSession : public FxSession
{
public:
	CCenterSession();
	virtual ~CCenterSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	bool				OnServerInfo(CCenterSession& refSession, google::protobuf::Message& refMsg);
private:
	char m_dataRecvBuf[1024 * 1024];
	CallBackDispatcher::ProtoCallBackDispatch<CCenterSession, CCenterSession> m_oProtoDispatch;
};

class CBinaryCenterSession : public CCenterSession
{
public:
	CBinaryCenterSession();
	~CBinaryCenterSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

class BinaryCenterSessionManager : public IFxSessionFactory
{
public:
	BinaryCenterSessionManager() {}
	virtual ~BinaryCenterSessionManager() {}

	virtual CBinaryCenterSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryCenterSession* pSession);

private:
	TDynamicPoolEx<CBinaryCenterSession> m_poolSessions;
};


#endif	//__CenterSession_H__
