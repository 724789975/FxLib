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
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	virtual void		Init();
	unsigned int		GetServerId() { return m_dwServerId; }

	virtual bool		OnServerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamMakeTeam(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamInviteTeam(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamChangeSlot(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamKickPlayer(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamGameStart(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamEnterTeam(CLoginSession& refSession, google::protobuf::Message& refMsg);
	bool		OnLoginRequestTeamPlayerLeave(CLoginSession& refSession, google::protobuf::Message& refMsg);

protected:
	unsigned int m_dwServerId;
	CallBackDispatcher::ProtoCallBackDispatch<CLoginSession, CLoginSession> m_oProtoDispatch;
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CBinaryLoginSession : public CLoginSession
{
public:
	CBinaryLoginSession();
	~CBinaryLoginSession();

	virtual bool		OnServerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg);
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void		Release(void);
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

	std::map<unsigned int, CBinaryLoginSession*>& GetLoginSessions() { return m_mapLoginSessions; }
	CBinaryLoginSession* GetLoginSession(unsigned int dwServerId);

private:
	TDynamicPoolEx<CBinaryLoginSession> m_poolSessions;
	std::map<unsigned int, CBinaryLoginSession*> m_mapLoginSessions;		//<serverid, login>
};


#endif	//__LoginSession_H__
