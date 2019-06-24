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
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	bool				OnGameNotifyGameManagerInfo(CServerSession& refSession, google::protobuf::Message& refMsg);
private:
	char m_dataRecvBuf[1024 * 1024];
	CallBackDispatcher::ProtoCallBackDispatch<CServerSession, CServerSession> m_oProtoDispatch;
};

class CBinaryServerSession : public CServerSession
{
public:
	CBinaryServerSession();
	~CBinaryServerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

class BinaryServerSessionManager : public IFxSessionFactory
{
public:
	BinaryServerSessionManager() {}
	virtual ~BinaryServerSessionManager() {}

	virtual CBinaryServerSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryServerSession* pSession);

private:
	TDynamicPoolEx<CBinaryServerSession> m_poolSessions;
};


#endif	//__ServerSession_H__
