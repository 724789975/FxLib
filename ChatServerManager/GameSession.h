#ifndef __GameSession_H__
#define __GameSession_H__

#include "lock.h"
#include "SocketSession.h"

class GameSession : public FxSession
{
public:
	GameSession();
	virtual ~GameSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

private:
	void				OnLogin(const char* pBuf, UINT32 dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

	std::string m_szServerId;
};

class GameSessionManager : public IFxSessionFactory
{
public:
	GameSessionManager() {}
	virtual ~GameSessionManager() {}

	virtual FxSession*	CreateSession();

	bool Init();
	void CloseSessions();

	virtual void Release(FxSession* pSession);
	virtual void Release(GameSession* pSession);

private:
	TDynamicPoolEx<GameSession> m_poolSessions;
	FxCriticalLock m_oLock;
};



#endif // !__CHatSession_H__
