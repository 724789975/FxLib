#ifndef __CHatSession_H__
#define __CHatSession_H__

#include <map>
#include "SocketSession.h"

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
	char m_dataRecvBuf[1024 * 1024];
private:
	char m_szId[32];
};

class ChatServerConnectedSession : public ChatServerSession
{
public:
	virtual void		OnClose(void);
protected:
private:
};

class ChatServerConnectSession : public ChatServerSession
{
public:
	virtual void		OnClose(void);
protected:
private:
};

class ChatServerSessionManager : public TSingleton<ChatServerSessionManager>
{
public:
	ChatServerSessionManager(){}
	virtual ~ChatServerSessionManager() {}

	ChatServerConnectSession*	CreateConnectSession();
	ChatServerConnectedSession*	CreateConnectedSession();

	void Init() {}
	virtual void Release(FxSession* pSession);


private:
	struct ChatServerSessionIpPort
	{
		unsigned int m_dwIP;
		unsigned int m_dwPort;
		ChatServerSession m_oSession;
	};

	std::map<unsigned char, ChatServerSessionIpPort*> m_mapSessionIpPort;

	IFxLock*			m_pLock;
};


#endif // !__CHatSession_H__
