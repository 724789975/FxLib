#ifndef _ChatGroup_H_
#define _ChatGroup_H_

#include <string>
#include <map>
#include "fxdb.h"
#include "chatdefine.h"

class DBLoadGroupQuery : public IQuery
{
public:
	DBLoadGroupQuery(unsigned int dwGroupId);
	~DBLoadGroupQuery() {}
	virtual int GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection);
	virtual void OnResult(void);
	virtual void Release(void);

	unsigned int m_dwGroupId;
	std::string m_strQuery;
	IDataReader* m_pReader;
};

class DBGroupChatQuery : public IQuery
{
public:
	DBGroupChatQuery(stCHAT_NOTIFY_CHAT_GROUP_CHAT& refChat);
	~DBGroupChatQuery() {}
	virtual INT32 GetDBId(void) { return 0; }
	virtual void OnQuery(IDBConnection *poDBConnection);
	virtual void OnResult(void);
	virtual void Release(void);

	std::string m_strQuery;
	stCHAT_NOTIFY_CHAT_GROUP_CHAT m_refGroupChat;
};

class ChatGroupMember
{
	friend class DBLoadGroupQuery;
	friend class DBInviteGroupMemberQuery;
public:
	enum ECHatPower
	{
		ECP_None = 0,
		ECP_CHAT = 1 << 0,

		//
		ECP_MANAGER = 1 << 29,
		ECP_OWNER = 1 << 30,
	};

	ChatGroupMember();
	virtual ~ChatGroupMember();

	bool CheckPower(ECHatPower ePower);

private:
	std::string m_szPlayerId;
	unsigned int m_dwPower;			//权限
	unsigned int m_dwJoinTime;		//加入时间
};

class ChatGroup
{
	friend class DBLoadGroupQuery;
	friend class DBInviteGroupMemberQuery;
public:
	ChatGroup();
	virtual ~ChatGroup();

	void OnGroupChat(stCHAT_NOTIFY_CHAT_GROUP_CHAT& refChat);

	ChatGroupMember* GetChatGroupMember(std::string szPlayerId);
	Protocol::EErrorCode RemoveChatMember(std::string szPlayer);
	void OnInviteMember(std::string szManager, std::string szPlayer);
	void OnLeaveGroupChat(std::string szPlayer);
	void OnLeaveGroupChatResult(Protocol::EErrorCode eErrorCode, std::string szPlayer);

private:
	std::map<unsigned int, std::map<std::string, ChatGroupMember> > m_mapChatGroupMembers;
	unsigned int m_dwGroupId;
	//std::map<std::string, ChatGroupMember> m_mapChatMembers;
};

class ChatGroupManager
{
	friend class DBLoadGroupQuery;
public:
	ChatGroupManager();
	virtual ~ChatGroupManager();

	bool Init();

	ChatGroup* GetChatGroup(unsigned int dwGroupId);
protected:
private:
	std::map<unsigned int, ChatGroup> m_mapChatGroups;
};

#endif // !_ChatGroup_H_
