#include "ChatGroup.h"
#include "ChatServer.h"
#include "utility.h"
#include "ChatPlayer.h"
#include "chatdefine.h"


//-------------------------------------------------------------
DBLoadGroupQuery::DBLoadGroupQuery(unsigned int dwGroupId)
{
	m_dwGroupId = dwGroupId;
	m_pReader = NULL;
	static char szTemp[512] = { 0 };
	memset(szTemp, 0, 512);
	sprintf(szTemp, "SELECT * FROM group_member_%d;", dwGroupId);
	m_strQuery = szTemp;
}

void DBLoadGroupQuery::OnQuery(IDBConnection *poDBConnection)
{
	if (poDBConnection->Query(m_strQuery.c_str(), &m_pReader) != FXDB_HAS_RESULT)
	{
		m_pReader->Release();
	}
}

void DBLoadGroupQuery::OnResult(void)
{
	ChatGroupManager& refManager = ChatServer::Instance()->GetChatGroupManager();
	while (m_pReader->GetNextRecord())
	{
		std::string szPlayerId = m_pReader->GetFieldValue(0);
		UINT32 dwPower = atoi(m_pReader->GetFieldValue(1));
		UINT32 dwJoinTime = atoi(m_pReader->GetFieldValue(2));
		//ChatGroupMember& refMember = refManager.m_mapChatGroups[m_dwGroupId].m_mapChatMembers[szPlayerId];
		ChatGroupMember& refMember = refManager.m_mapChatGroups[m_dwGroupId].m_mapChatGroupMembers[HashToIndex(szPlayerId.c_str(), szPlayerId.size())][szPlayerId];
		refMember.m_szPlayerId = szPlayerId;
		refMember.m_dwPower = dwPower;
		refMember.m_dwJoinTime = dwJoinTime;
	}
}

void DBLoadGroupQuery::Release(void)
{
	m_pReader->Release();
	delete this;
}

DBGroupChatQuery::DBGroupChatQuery(stCHAT_NOTIFY_CHAT_GROUP_CHAT& refChat)
{
	m_refGroupChat = refChat;
	static char szTemp[2048 * 3 + 1] = { 0 };
	static char szContentEacape[2048 * 3 + 1] = { 0 };
	memset(szTemp, 0, 2048);
	memset(szContentEacape, 0, 2048 * 3 + 1);
	mysql_escape_string(szContentEacape, m_refGroupChat.szContent.c_str(), m_refGroupChat.szContent.size());
	sprintf(szTemp, "INSERT INTO group_chat_%d "
		"(`sender_id`, `chat_type`, `content`, `send_time`) "
		"VALUES('%s', %d, '%s', %d);",
		m_refGroupChat.dwGroupId, m_refGroupChat.szSenderId.c_str(), m_refGroupChat.eChatType, m_refGroupChat.szContent.c_str(), m_refGroupChat.dwSendTime);
	m_strQuery = szTemp;
}

void DBGroupChatQuery::OnQuery(IDBConnection *poDBConnection)
{
	if (poDBConnection->Query(m_strQuery.c_str()) != FXDB_HAS_RESULT)
	{
	}
}

void DBGroupChatQuery::OnResult(void)
{
	//m_refGroupChat.dwGroupId
	ChatGroup* pChatGroup = ChatServer::Instance()->GetChatGroupManager().GetChatGroup(m_refGroupChat.dwGroupId);
	if (!pChatGroup)
	{
		LogExe(LogLv_Critical, "can't find group : %d", m_refGroupChat.dwGroupId);
		return;
	}
	pChatGroup->OnGroupChat(m_refGroupChat);
}

void DBGroupChatQuery::Release(void)
{
	delete this;
}

//-------------------------------------------------------------
ChatGroupMember::ChatGroupMember()
{

}

ChatGroupMember::~ChatGroupMember()
{

}

bool ChatGroupMember::CheckPower(ECHatPower ePower)
{
	return m_dwPower & ePower;
}

//-------------------------------------------------------------
ChatGroup::ChatGroup()
{
}

ChatGroup::~ChatGroup()
{
}

void ChatGroup::OnGroupChat(stCHAT_NOTIFY_CHAT_GROUP_CHAT& refChat)
{
	stCHAT_NOTIFY_PLAYER_GROUP_CHAT oChat;
	oChat.dwGroupId = refChat.dwGroupId;
	oChat.szSenderId = refChat.szSenderId;
	oChat.eChatType = refChat.eChatType;
	oChat.szContent = refChat.szContent;
	oChat.dwSendTime = refChat.dwSendTime;
	for (std::map<unsigned int, std::map<std::string, ChatGroupMember> >::iterator it = m_mapChatGroupMembers.begin();
		it != m_mapChatGroupMembers.end(); ++it)
	{
		if (ChatServer::Instance()->CheckHashIndex(it->first))
		{
			for (std::map<std::string, ChatGroupMember>::iterator itMember = it->second.begin();
				itMember != it->second.end(); ++itMember)
			{
				ChatPlayer* pPlayer = ChatServer::Instance()->GetChatPlayerManager().GetChatPlayer(itMember->first);
				if (pPlayer)
				{
					pPlayer->OnGroupChat(oChat);
				}
			}
		}
		else
		{
			ChatServerSessionManager& refServerSessionManager = ChatServer::Instance()->GetChatServerSessionManager();
			ChatServerSession* pChatServerSession = refServerSessionManager.GetChatServerSession(HashToIndex(it->first));
			if (!pChatServerSession)
			{
				LogExe(LogLv_Critical, "cant't find chat server session");
				continue;
			}
			stCHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT oMemberChat;
			oMemberChat.oChat = oChat;
			for (std::map<std::string, ChatGroupMember>::iterator itMember = it->second.begin();
				itMember != it->second.end(); ++itMember)
			{
				oMemberChat.vecPlayerIds.push_back(itMember->first);
			}

			pChatServerSession->OnGroupMemberChat(oMemberChat);
		}
	}
}

ChatGroupMember* ChatGroup::GetChatGroupMember(std::string szPlayerId)
{
	unsigned int dwHashIndex = HashToIndex(szPlayerId.c_str(), szPlayerId.size());
	if (m_mapChatGroupMembers.find(dwHashIndex) == m_mapChatGroupMembers.end())
	{
		return NULL;
	}
	if (m_mapChatGroupMembers[dwHashIndex].find(szPlayerId) == m_mapChatGroupMembers[dwHashIndex].end())
	{
		return NULL;
	}
	return &m_mapChatGroupMembers[dwHashIndex][szPlayerId];
}

class DBInviteGroupMemberQuery : public IQuery
{
public:
	DBInviteGroupMemberQuery()
	{
	}
	~DBInviteGroupMemberQuery() {}
	virtual INT32 GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		if (poDBConnection->Query(m_strQuery.c_str()) != FXDB_SUCCESS)
		{
		}
	}

	virtual void OnResult(void)
	{
		//todo
	}

	virtual void Release(void)
	{
		delete this;
	}
	std::string m_strQuery;
};

void ChatGroup::InviteMember(std::string szManager, std::string szPlayer)
{
	Protocol::EErrorCode eErrorCode = Protocol::EEC_NONE;
	if (GetChatGroupMember(szPlayer))
	{
		eErrorCode = Protocol::EEC_AlreadyInChatGroup;
	}
	ChatGroupMember* pManager = GetChatGroupMember(szManager);
	if (pManager->CheckPower(ChatGroupMember::ECP_MANAGER))
	{
		eErrorCode = Protocol::EEC_PermissionDenied;
	}
	if (eErrorCode != Protocol::EEC_NONE)
	{
		//todo
		return;
	}

	DBInviteGroupMemberQuery* pQuery = new DBInviteGroupMemberQuery;
	FxDBGetModule()->AddQuery(pQuery);
}

//-------------------------------------------------------------
class DBLoadGroupsQuery : public IQuery
{
public:
	DBLoadGroupsQuery()
	{
		m_pReader = NULL;
		m_strQuery = "SELECT `id` FROM groups;";
	}
	~DBLoadGroupsQuery(){}
	virtual INT32 GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		if (poDBConnection->Query(m_strQuery.c_str(), &m_pReader) != FXDB_HAS_RESULT)
		{
			m_pReader->Release();
		}
	}

	virtual void OnResult(void)
	{
		UINT32 dwGroupId = 0;
		while (m_pReader->GetNextRecord())
		{
			dwGroupId = atoi(m_pReader->GetFieldValue(0));

			if (ChatServer::Instance()->CheckHashIndex(HashToIndex(dwGroupId)))
			{
				DBLoadGroupQuery* pQuery = new DBLoadGroupQuery(dwGroupId);
				FxDBGetModule()->AddQuery(pQuery);
			}
		}
	}

	virtual void Release(void)
	{
		m_pReader->Release();
		delete this;
	}
	std::string m_strQuery;
	IDataReader* m_pReader;
};

ChatGroupManager::ChatGroupManager()
{

}

ChatGroupManager::~ChatGroupManager()
{

}

bool ChatGroupManager::Init()
{
	DBLoadGroupsQuery* pQuery = new DBLoadGroupsQuery();
	FxDBGetModule()->AddQuery(pQuery);
	return true;
}

ChatGroup* ChatGroupManager::GetChatGroup(unsigned int dwGroupId)
{
	if (m_mapChatGroups.find(dwGroupId) == m_mapChatGroups.end())
	{
		return NULL;
	}
	return &m_mapChatGroups[dwGroupId];
}

