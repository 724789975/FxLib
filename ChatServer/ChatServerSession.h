#ifndef __CHatServerSession_H__
#define __CHatServerSession_H__

#include <map>
#include "lock.h"
#include "SocketSession.h"
#include "chatdefine.h"

class ChatServerSession : public FxSession
{
public:
	ChatServerSession();
	virtual ~ChatServerSession();

	virtual void		OnConnect(void);

	virtual void		OnClose(void);

	virtual void		OnError(UINT32 dwErrorNo);

	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);

	virtual void		Release(void);

	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }

	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];
private:
	char m_szId[32];
};

class ChatServerSessionManager : public TSingleton<ChatServerSessionManager>, public IFxSessionFactory
{
public:
	ChatServerSessionManager(){}
	virtual ~ChatServerSessionManager() {}

	virtual FxSession*	CreateSession();

	void Init() {}
	virtual void Release(FxSession* pSession);
	
private:
	std::map<unsigned char, ChatServerSession*> m_mapSessionIpPort;

	ChatServerSession m_oChatServerSessions[ChatConstant::g_dwChatServerNum- 1];

	FxCriticalLock m_oLock;
};


#endif // !__CHatServerSession_H__
