#ifndef __CHatSession_H__
#define __CHatSession_H__

#include "lock.h"
#include "SocketSession.h"
#include "chatdefine.h"

class ChatSession : public FxSession
{
public:
	ChatSession();
	virtual ~ChatSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

	void				Reset();

	void				OnMsg(const char* pBuf, UINT32 dwLen);
	void				OnLogin(const char* pBuf, UINT32 dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

	char m_szId[IDLENTH];
};

class ChatSessionManager : public IFxSessionFactory
{
public:
	ChatSessionManager(){}
	virtual ~ChatSessionManager() {}

	virtual FxSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(ChatSession* pSession);

private:
	TDynamicPoolEx<ChatSession> m_poolSessions;

	FxCriticalLock m_oLock;
};

#endif // !__CHatSession_H__
