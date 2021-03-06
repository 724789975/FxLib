#ifndef __TeamSession_H__
#define __TeamSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "proto_dispatcher.h"

class CTeamSession : public FxSession
{
public:
	CTeamSession();
	virtual ~CTeamSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	virtual void		Init();
	unsigned int		GetServerId() { return m_dwServerId; }

	virtual bool		OnServerInfo(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginMakeTeam(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamNotifyLoginTeamInfo(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginInviteTeam(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginChangeSlot(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginKickPlayer(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginGameStart(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginEnterTeam(CTeamSession& refSession, google::protobuf::Message& refMsg);
	bool		OnTeamAckLoginPlayerLeave(CTeamSession& refSession, google::protobuf::Message& refMsg);

protected:
	unsigned int m_dwServerId;
	CallBackDispatcher::ProtoCallBackDispatch<CTeamSession, CTeamSession> m_oProtoDispatch;
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CBinaryTeamSession : public CTeamSession
{
public:
	CBinaryTeamSession();
	~CBinaryTeamSession();

	virtual bool		OnServerInfo(CTeamSession& refSession, google::protobuf::Message& refMsg);
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void		Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

class BinaryTeamSessionManager : public IFxSessionFactory
{
public:
	BinaryTeamSessionManager() {}
	virtual ~BinaryTeamSessionManager() {}

	virtual CBinaryTeamSession* CreateSession();
	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CBinaryTeamSession* pSession);

	std::map<unsigned int, CBinaryTeamSession*>& GetTeamSessions() { return m_mapTeamSessions; }
	CBinaryTeamSession* GetTeamSession(unsigned int dwTeamServerId);

private:
	TDynamicPoolEx<CBinaryTeamSession> m_poolSessions;
	std::map<unsigned int, CBinaryTeamSession*> m_mapTeamSessions;		//<serverid, login>
};


#endif	//__TeamSession_H__
