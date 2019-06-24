#ifndef __ServerSession_H__
#define __ServerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"
#include "lock.h"

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

	virtual void		Init();

	bool				OnServerInfo(CServerSession& refSession, google::protobuf::Message& refMsg);
private:
	char				m_dataRecvBuf[1024 * 1024];
	CallBackDispatcher::ProtoCallBackDispatch<CServerSession, CServerSession> m_oProtoDispatch;

	//服务器的相关信息在这个类里就可以了 这样就简单了一些 服务器数量不会太多
	unsigned int				m_dwServerId;
	//std::string			m_szTeamServerIp;
	unsigned int				m_dwTeamPort;
	//std::string			m_szGameServerManagerIp;
	unsigned int				m_dwGameServerManagerPort;
	//std::string			m_szLoginServerIp;
	unsigned int				m_dwLoginPort;
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

	std::set<CBinaryServerSession*>& GetSessions() { return m_setSessions; }

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryServerSession* pSession);

private:
	std::set<CBinaryServerSession*> m_setSessions;
	TDynamicPoolEx<CBinaryServerSession> m_poolSessions;
};


#endif	//__ServerSession_H__
