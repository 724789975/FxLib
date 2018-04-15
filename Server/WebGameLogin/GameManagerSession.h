#ifndef __GameManagerSession_H__
#define __GameManagerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"

class CGameManagerSession : public FxSession
{
public:
	CGameManagerSession();
	virtual ~CGameManagerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	virtual void		Init();
	unsigned int		GetServerId() { return m_dwServerId; }

	virtual bool		OnServerInfo(CGameManagerSession& refSession, google::protobuf::Message& refMsg);

protected:
	unsigned int m_dwServerId;
	CallBackDispatcher::ProtoCallBackDispatch<CGameManagerSession, CGameManagerSession> m_oProtoDispatch;
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CBinaryGameManagerSession : public CGameManagerSession
{
public:
	CBinaryGameManagerSession();
	~CBinaryGameManagerSession();

	virtual bool		OnServerInfo(CGameManagerSession& refSession, google::protobuf::Message& refMsg);
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void		Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

class BinaryGameManagerSessionManager : public IFxSessionFactory
{
public:
	BinaryGameManagerSessionManager() {}
	virtual ~BinaryGameManagerSessionManager() {}

	virtual CBinaryGameManagerSession* CreateSession();
	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryGameManagerSession* pSession);

	std::map<unsigned int, CBinaryGameManagerSession*>& GetGameManagerSessions() { return m_mapGameManagerSessions; }

private:
	TDynamicPoolEx<CBinaryGameManagerSession> m_poolSessions;
	std::map<unsigned int, CBinaryGameManagerSession*> m_mapGameManagerSessions;		//<serverid, login>
};


#endif	//__GameManagerSession_H__
