#ifndef __GMSession_H__
#define __GMSession_H__

#include <map>
#include "lock.h"
#include "SocketSession.h"
#include "chatdefine.h"

class GMSession : public FxSession
{
public:
	GMSession();
	virtual ~GMSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oTextDataHeader; }

private:
	TextDataHeader m_oTextDataHeader;
	char m_dataRecvBuf[1024 * 1024];
};

class GMSessionManager : public IFxSessionFactory
{
public:
	GMSessionManager() {}
	virtual ~GMSessionManager() {}

	virtual FxSession*	CreateSession();

	bool Init() { return true; };

	virtual void Release(FxSession* pSession);

private:
	GMSession m_oGMSession;

	FxCriticalLock m_oLock;
};

#endif	//!__GMSession_H__
