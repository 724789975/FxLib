#ifndef __ChatPlayer_H__
#define __ChatPlayer_H__

#include "ChatSession.h"
#include "chatdefine.h"

class ChatPlayer
{
public:
	ChatPlayer();
	virtual ~ChatPlayer();

	bool Init(ChatSession* pSession, std::string szID);
	ChatSession* GetSession() { return m_pSession; }

	void OnMsg(const char* pBuf, unsigned int dwLen);

	void OnPrivateChat(const char* pBuf, unsigned int dwLen);

	void OnPrivateChat(const char* szSender, const Protocol::EChatType eChatType, const std::string szContent, unsigned int dwTimeStamp);

	void OnRequestCreateChatGroup(const char* pBuf, unsigned int dwLen);
	void OnCreateChatGroup(unsigned int dwGroupId);
	void OnRequestGroupChat(const char* pBuf, unsigned int dwLen);
	void OnGroupChat(stCHAT_NOTIFY_PLAYER_GROUP_CHAT& refChat);
	void OnRequestInviteEnterGroupChat(const char* pBuf, unsigned int dwLen);
	void OnInviteEnterGroupChatResult(stCHAT_ACK_PLAYER_INVITE_GROUP_CHAT& refInviteResult);
	void OnRequestLeaveGroupChat(const char* pBuf, unsigned int dwLen);
	void OnLeaveGroupChatResult(stCHAT_ACK_PLAYER_LEAVE_GROUP_CHAT& refLeaveChatResult);
private:
	ChatSession* m_pSession;

	char m_szPyayerId[IDLENTH];
};

#endif // !__ChatPlayer_H__

