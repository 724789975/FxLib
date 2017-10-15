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

	void				OnGroupCreate(unsigned int dwGroupId);

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

	UINT32 m_dwHashIndex;
private:
	char m_szId[32];
private:
	void OnChatToChatHashIndex(const char* pBuf, UINT32 dwLen);
	void OnChatToChatPrivateChat(const char* pBuf, UINT32 dwLen);
	void OnChatToChatGroupCreate(const char* pBuf, UINT32 dwLen);
	void OnChatToChatGroupChat(const char* pBuf, UINT32 dwLen);
};

class ChatServerSessionManager : public IFxSessionFactory
{
public:
	ChatServerSessionManager(){}
	virtual ~ChatServerSessionManager() {}

	virtual FxSession*	CreateSession();

	ChatServerSession* GetChatServerSession(unsigned int dwIndex);

	bool Init() { return true; }
	virtual void Release(FxSession* pSession);

	void SetHashIndex(UINT32 dwIndex, ChatServerSession* pChatServerSession);

	void CloseSessions();
	
private:
	std::map<unsigned int, ChatServerSession*> m_mapSessionIpPort;

	ChatServerSession m_oChatServerSessions[ChatConstant::g_dwChatServerNum- 1];

	FxCriticalLock m_oLock;

};


#endif // !__CHatServerSession_H__
