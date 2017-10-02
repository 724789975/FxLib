#ifndef __CHatSession_H__
#define __CHatSession_H__

#include <map>
#include "lock.h"
#include "SocketSession.h"
#include "chatdefine.h"

class ChatServerSession : public FxSession
{
	friend class ChatServerSessionManager;
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

	UINT32 GetChatServerPort() { return m_dwChatServerPort; }
	UINT32 GetChatPort() { return m_dwChatPort; }
	UINT32 GetWebSocketChatPort() { return m_dwWebSocketChatPort; }
	std::string GetChatIp() { return m_szChatIp; }

	void ChatLogin(std::string szPlayerId);
	void ChatLoginByGM(std::string szPlayerId);

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[1024 * 1024];
private:
	UINT32 m_dwChatServerPort;
	UINT32 m_dwChatPort;
	UINT32 m_dwWebSocketChatPort;
	std::string m_szChatIp;
private:
	void OnChatServerInfo(const char* pBuf, UINT32 dwLen);
	void OnChatLoginSign(const char* pBuf, UINT32 dwLen);
	void OnChatLoginSignGM(const char* pBuf, UINT32 dwLen);
};

class ChatServerSessionManager : public IFxSessionFactory
{
public:
	ChatServerSessionManager() {}
	virtual ~ChatServerSessionManager() {}

	virtual FxSession*	CreateSession();

	bool Init() { return true; };
	void CloseSessions();

	virtual void Release(FxSession* pSession);
	ChatServerSession* GetChatServerSessions() { return m_oChatServerSessions; }
	ChatServerSession* GetChatServerSession(UINT32 dwHashIndex);

	void OnChatServerInfo(ChatServerSession* pChatServerSession);

	void BroadcastMsg(const Protocol::EChatType& eChatType, const std::string& szContent);
private:
	std::map<unsigned int, ChatServerSession*> m_mapSessionIpPort;

	ChatServerSession m_oChatServerSessions[ChatConstant::g_dwChatServerNum];

	FxCriticalLock m_oLock;
};


#endif // !__CHatSession_H__
