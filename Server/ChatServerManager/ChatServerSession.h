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
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

	unsigned int GetChatServerPort() { return m_dwChatServerPort; }
	unsigned int GetChatPort() { return m_dwChatPort; }
	unsigned int GetWebSocketChatPort() { return m_dwWebSocketChatPort; }
	std::string GetChatIp() { return m_szChatIp; }

	void ChatLogin(std::string szServerId, std::string szPlayerId);
	void ChatLoginByGM(std::string szPlayerId);

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[1024 * 1024];
private:
	unsigned int m_dwChatServerPort;
	unsigned int m_dwChatPort;
	unsigned int m_dwWebSocketChatPort;
	std::string m_szChatIp;
private:
	void OnChatServerInfo(const char* pBuf, unsigned int dwLen);
	void OnChatLoginSign(const char* pBuf, unsigned int dwLen);
	void OnChatLoginSignGM(const char* pBuf, unsigned int dwLen);
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
	ChatServerSession* GetChatServerSession(unsigned int dwHashIndex);

	void OnChatServerInfo(ChatServerSession* pChatServerSession);

	void BroadcastMsg(const Protocol::EChatType& eChatType, const std::string& szContent);
private:
	std::map<unsigned int, ChatServerSession*> m_mapSessionIpPort;

	ChatServerSession m_oChatServerSessions[CHAT_SERVER_NUM];

	FxCriticalLock m_oLock;
};


#endif // !__CHatSession_H__
