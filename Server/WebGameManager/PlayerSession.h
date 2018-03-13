#ifndef __PlayerSession_H__
#define __PlayerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

#include "msg_proto/web_game.pb.h"

struct stGAME_NOTIFY_GAME_MANAGER_INFO;
class CPlayerSession : public FxSession
{
public:
	CPlayerSession();
	virtual ~CPlayerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	void				OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen);

	//void				OnGameInfo(stGAME_NOTIFY_GAME_MANAGER_INFO& refInfo);
	void				OnGameInfo(GameProto::GameNotifyGameManagerInfo& refInfo);
private:
	char m_dataRecvBuf[1024 * 1024];
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

	virtual FxSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(CWebSocketPlayerSession* pSession);

private:
	TDynamicPoolEx<CWebSocketPlayerSession> m_poolSessions;
};


#endif	//__PlayerSession_H__
