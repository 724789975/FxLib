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

	void OnMsg(const char* pBuf, UINT32 dwLen);

	void OnPrivateChat(const char* pBuf, UINT32 dwLen);

	void OnPrivateChat(const char* szSender, const Protocol::EChatType eChatType, const std::string szContent, unsigned int dwTimeStamp);

	void OnRequestCreateChatGroup(const char* pBuf, UINT32 dwLen);
	void OnCreateChatGroup(UINT32 dwGroupId);
	void OnRequestGroupChat(const char* pBuf, UINT32 dwLen);
	void OnGroupChat(stCHAT_NOTIFY_PLAYER_GROUP_CHAT& refChat);
	void OnRequestInviteEnterGroupChat(const char* pBuf, UINT32 dwLen);
	void OnRequestLeaveGroupChat(const char* pBuf, UINT32 dwLen);
	void OnLeaveGroupChatResult(stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT_RESULT& refLeaveChatResult);
private:
	ChatSession* m_pSession;

	char m_szPyayerId[IDLENTH];
};

#endif // !__ChatPlayer_H__

