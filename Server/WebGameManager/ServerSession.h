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

	virtual FxSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryServerSession* pSession);

private:
	TDynamicPoolEx<CBinaryServerSession> m_poolSessions;
};


#endif	//__ServerSession_H__
