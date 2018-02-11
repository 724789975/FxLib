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
	virtual void		Release(void) = 0;
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() = 0;

	void				Reset();

	void				OnMsg(const char* pBuf, UINT32 dwLen);
	void				OnLogin(const char* pBuf, UINT32 dwLen);
private:
	
	char m_dataRecvBuf[64 * 1024];

	char m_szId[IDLENTH];
};

class ChatBinarySession : public ChatSession
{
public:
	virtual void		Release(void);
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
protected:
private:
	BinaryDataHeader m_oBinaryDataHeader;
};

class ChatBinarySessionManager : public IFxSessionFactory
{
public:
	ChatBinarySessionManager(){}
	virtual ~ChatBinarySessionManager() {}

	virtual FxSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(ChatBinarySession* pSession);

private:
	TDynamicPoolEx<ChatBinarySession> m_poolSessions;
};

class ChatWebSocketSession : public ChatSession
{
public:
	virtual void		Release(void);
	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
protected:
private:
	WebSocketDataHeader m_oWebSocketDataHeader;
};

class ChatWebSocketSessionManager : public IFxSessionFactory
{
public:
	ChatWebSocketSessionManager() {}
	virtual ~ChatWebSocketSessionManager() {}

	virtual FxSession* CreateSession();

	bool Init();
	virtual void Release(FxSession* pSession);
	void Release(ChatWebSocketSession* pSession);

private:
	TDynamicPoolEx<ChatWebSocketSession> m_poolSessions;
};

#endif // !__CHatSession_H__
