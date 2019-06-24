#ifndef __PlayerSession_H__
#define __PlayerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"
#include "msg_proto/web_game.pb.h"

class CPlayerSession : public FxSession
{
public:
	CPlayerSession();
	virtual ~CPlayerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	virtual void		Init();

	bool		OnPlayerRequestLoginServerId(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLogin(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginMakeTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginInviteTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginChangeSlot(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginGameStart(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginOnLinePlayer(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginEnterTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginRefuseEnterTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg);
	bool		OnPlayerRequestLoginLeaveTeam(CPlayerSession& refSession, google::protobuf::Message& refMsg);

protected:
	unsigned long long m_qwPlayerId;
private:
	char m_dataRecvBuf[1024 * 1024];

	CallBackDispatcher::ProtoCallBackDispatch<CPlayerSession, CPlayerSession> m_oProtoDispatch;
};

class CWebSocketPlayerSession : public CPlayerSession
{
public:
	CWebSocketPlayerSession(){}
	~CWebSocketPlayerSession(){}

	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
	virtual void Release(void);
private:
	WebSocketDataHeader m_oWebSocketDataHeader;

};

class WebSocketPlayerSessionManager : public IFxSessionFactory
{
public:
	WebSocketPlayerSessionManager() {}
	virtual ~WebSocketPlayerSessionManager() {}

	virtual CWebSocketPlayerSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CWebSocketPlayerSession* pSession);

private:
	TDynamicPoolEx<CWebSocketPlayerSession> m_poolSessions;
};


#endif	//__PlayerSession_H__
