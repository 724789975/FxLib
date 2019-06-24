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
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

	void				OnGroupCreate(unsigned int dwGroupId);
	void				OnGroupMemberChat(stCHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT& refChat);
	void				OnInviteGroupMember(stCHAT_NOTIFY_CHAT_INVITE_ENTER_GROUP_CHAT& refChat);
	void				OnInviteGroupMemberResult(stCHAT_NOTIFY_CHAT_INVITE_ENTER_GROUP_CHAT_RESULT& refInviteChatResult);
	void				OnLeaveGroupChat(stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT& refLeaveGroupChat);
	void				OnLeaveGroupChatResult(stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT_RESULT& refLeaveGroupChatResult);

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

	unsigned int m_dwHashIndex;
private:
	char m_szId[32];
private:
	void OnChatToChatHashIndex(const char* pBuf, unsigned int dwLen);
	void OnChatToChatPrivateChat(const char* pBuf, unsigned int dwLen);
	void OnChatToChatGroupCreate(const char* pBuf, unsigned int dwLen);
	void OnChatToChatGroupChat(const char* pBuf, unsigned int dwLen);
	void OnChatToChatGroupMemberChat(const char* pBuf, unsigned int dwLen);
	void OnChatToChatInviteGroupMember(const char* pBuf, unsigned int dwLen);
	void OnChatToChatInviteGroupMemberResult(const char* pBuf, unsigned int dwLen);
	void OnChatToChatLeaveGroupChat(const char* pBuf, unsigned int dwLen);
	void OnChatToChatLeaveGroupChatResult(const char* pBuf, unsigned int dwLen);
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

	void SetHashIndex(unsigned int dwIndex, ChatServerSession* pChatServerSession);

	void CloseSessions();
	
private:
	std::map<unsigned int, ChatServerSession*> m_mapSessionIpPort;

#if CHAT_SERVER_NUM - 1
	ChatServerSession m_oChatServerSessions[CHAT_SERVER_NUM - 1];
#endif

	FxCriticalLock m_oLock;

};


#endif // !__CHatServerSession_H__
