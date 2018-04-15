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
	unsigned int		GetServerId() { return m_dwServerId; }

	virtual bool		OnServerInfo(CLoginSession& refSession, google::protobuf::Message& refMsg);
	virtual bool		OnLoginRequestTeamMakeTeam(CLoginSession& refSession, google::protobuf::Message& refMsg);
	virtual bool		OnLoginRequestTeamInviteTeam(CLoginSession& refSession, google::protobuf::Message& refMsg);
	virtual bool		OnLoginRequestTeamChangeSlot(CLoginSession& refSession, google::protobuf::Message& refMsg);

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

private:
	TDynamicPoolEx<CBinaryLoginSession> m_poolSessions;
	std::map<unsigned int, CBinaryLoginSession*> m_mapLoginSessions;		//<serverid, login>
};


#endif	//__LoginSession_H__
