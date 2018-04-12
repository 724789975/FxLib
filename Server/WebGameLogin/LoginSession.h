#ifndef __LoginSession_H__
#define __LoginSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"

class CLoginSession : public FxSession
{
public:
	CLoginSession();
	virtual ~CLoginSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	virtual void		Init();

	bool				OnGameNotifyGameManagerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg);
private:
	char m_dataRecvBuf[1024 * 1024];
	CallBackDispatcher::ProtoCallBackDispatch<CLoginSession, CLoginSession> m_oProtoDispatch;
	unsigned int m_dwServerId;
};

class CBinaryLoginSession : public CLoginSession
{
public:
	CBinaryLoginSession();
	~CBinaryLoginSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

class BinaryLoginSessionManager : public IFxSessionFactory
{
public:
	BinaryLoginSessionManager() {}
	virtual ~BinaryLoginSessionManager() {}

	virtual CBinaryLoginSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryLoginSession* pSession);

private:
	TDynamicPoolEx<CBinaryLoginSession> m_poolSessions;
};


#endif	//__LoginSession_H__
