#ifndef __ChatDefine_H__
#define __ChatDefine_H__

#include <vector>
#include "netstream.h"
#include "fxdb.h"

#define IDLENTH 64

namespace ChatConstant
{
	static const unsigned int g_dwChatServerNum = 2;
	static const unsigned int g_dwHashGen = 256;
}

namespace Protocol
{
	enum EErrorCode
	{
		EEC_NONE = 0,
		EEC_PermissionDenied,
		EEC_AlreadyInChatGroup,
	};
	enum EChatType
	{
		ECT_NONE,
		ECT_String,
		ECT_Image,
		ECT_Voice,
	};
	enum EChatProtocol
	{
		//chat<--->chat 10001 11000
		CHAT_TO_CHAT_BEGIN = 10001,
		CHAT_NOTIFY_CHAT_HASH_INDEX = 10002,
		CHAT_NOTIFY_CHAT_PRIVATE_CHAT,
		CHAT_NOTIFY_CHAT_GROUP_CREATE,
		CHAT_NOTIFY_CHAT_GROUP_CHAT,
		CHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT,
		CHAT_NOTIFY_CHAT_INVITE_ENTER_GROUP_CHAT,
		CHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT,
		CHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT_RESULT,
		CHAT_TO_CHAT_END = 11000,

		//chat --->chatmanager 20001 25000
		CHAT_TO_CHAT_MANAGER_BEGIN = 20001,
		CHAT_NOTIFY_CHAT_MANAGER_INFO,
		CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN,
		CHAT_NOTIFY_CHAT_MANAGER_LOGIN_SIGN_GM,
		CHAT_TO_CHAT_MANAGER_END = 25000,

		//chatmanager --->chat 25001 29999
		CHAT_MANAGER_TO_CHAT_BEGIN = 25001,
		CHAT_MANAGER_NOTIFY_CHAT_INFO,
		CHAT_MANAGER_NOTIFY_CHAT_BROADCAST,
		CHAT_MANAGER_NOTIFY_CHAT_LOGIN,
		CHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM,		//Í¨¹ýGMµÇÂ¼µÄ
		CHAT_MANAGER_TO_CHAT_END = 29999,

		//player--->chat 30000 35000
		PLAYER_CHAT_BEGIN = 30000,
		PLAYER_REQUEST_CHAT_LOGIN,
		PLAYER_REQUEST_PRIVATE_CHAT,
		PLAYER_REQUEST_CREATE_CHAT_GROUP,
		PLAYER_REQUEST_CHAT_GROUP_CHAT,
		PLAYER_REQUEST_INVITE_ENTER_GROUP_CHAT,
		PLAYER_REQUEST_LEAVE_GROUP_CHAT,
		PLAYER_CHAT_END = 34999,

		//chat--->player 35000 35999
		CHAT_PLAYER_BEGIN = 35000,
		CHAT_ACK_PLAYER_LOGIN,
		CHAT_NOTIFY_PLAYER_PRIVATE_CHAT,
		CHAT_ACK_PLAYER_CREATE_CHAT_GROUP,
		CHAT_NOTIFY_PLAYER_GROUP_CHAT,
		CHAT_NOTIFY_PLAYER_LEAVE_GROUP_CHAT,
		CHAT_PLAYER_END = 39999,

		//game--->chatmanager 40000 44999
		GAME_CHAT_MANAGER_BEGIN = 40000,
		GAME_REQUEST_CHAT_MANAGER_LOGIN,
		GAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN,
		GAME_CHAT_MANAGER_END = 44999,

		//chatmanager--->game 45000 49999
		CHAT_MANAGER_GAME_BEGIN = 45000,
		CHAT_MANAGER_ACK_GAME_LOGIN,
		CHAT_MANAGER_ACK_GAME_PLAYER_LOGIN,
		CHAT_MANAGER_GAME_END = 49999,
	};
}

//----------------------------------------------------------------------
struct stCHAT_TO_CHAT_HASH_INDEX
{
	unsigned int dwHashIndex;
	bool Write(CNetStream& refStream)
	{
		return refStream.WriteInt(dwHashIndex);
	}
	bool Read(CNetStream& refStream)
	{
		return refStream.ReadInt(dwHashIndex);
	}
};

struct stCHAT_NOTIFY_CHAT_GROUP_CREATE
{
	unsigned int dwGroupId;
	bool Write(CNetStream& refStream)
	{
		return refStream.WriteInt(dwGroupId);
	}
	bool Read(CNetStream& refStream)
	{
		return refStream.ReadInt(dwGroupId);
	}
};

struct stCHAT_NOTIFY_CHAT_GROUP_CHAT
{
	unsigned int dwGroupId;
	std::string szSenderId;
	Protocol::EChatType eChatType;
	std::string szContent;
	unsigned int dwSendTime;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		if (!refStream.WriteString(szSenderId))	return false;
		if (!refStream.WriteInt((unsigned int&)eChatType)) return false;
		if (!refStream.WriteString(szContent)) return false;
		if (!refStream.WriteInt(dwSendTime)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadString(szSenderId))	return false;
		if (!refStream.ReadInt((unsigned int&)eChatType)) return false;
		if (!refStream.ReadString(szContent)) return false;
		if (!refStream.ReadInt(dwSendTime)) return false;
		return true;
	}
};

struct stCHAT_NOTIFY_PLAYER_GROUP_CHAT : public stCHAT_NOTIFY_CHAT_GROUP_CHAT
{
};
struct stCHAT_NOTIFY_CHAT_GROUP_MEMBER_CHAT
{
	std::vector<std::string> vecPlayerIds;
	stCHAT_NOTIFY_PLAYER_GROUP_CHAT oChat;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt((unsigned int)vecPlayerIds.size())) return false;
		for (std::vector<std::string>::iterator it = vecPlayerIds.begin();
			it != vecPlayerIds.end(); ++it)
		{
			if (!refStream.WriteString(*it)) return false;
		}
		if (!oChat.Write(refStream)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		unsigned int dwLen = 0;
		if (!refStream.ReadInt(dwLen)) return false;
		for (unsigned int i = 0; i < dwLen; ++i)
		{
			std::string szPlayerId;
			if (!refStream.ReadString(szPlayerId))	return false;
			vecPlayerIds.push_back(szPlayerId);
		}
		if (!oChat.Read(refStream)) return false;
		return true;
	}
};

struct stCHAT_NOTIFY_CHAT_INVITE_ENTER_GROUP_CHAT
{
	unsigned int dwGroupId;
	std::string szPlayerId;
	std::string szInviter;
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadString(szPlayerId)) return false;
		if (!refStream.ReadString(szInviter)) return false;
		return true;
	}
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		if (!refStream.WriteString(szPlayerId)) return false;
		if (!refStream.WriteString(szInviter)) return false;
		return true;
	}
};

struct stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT
{
	unsigned int dwGroupId;
	std::string szPlayerId;
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadString(szPlayerId)) return false;
		return true;
	}
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		if (!refStream.WriteString(szPlayerId)) return false;
		return true;
	}
};

struct stCHAT_NOTIFY_CHAT_PLAYER_LEAVE_GROUP_CHAT_RESULT
{
	unsigned int dwGroupId;
	std::string szPlayerId;
	unsigned int dwResult;
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadString(szPlayerId)) return false;
		if (!refStream.ReadInt(dwResult)) return false;
		return true;
	}
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		if (!refStream.WriteString(szPlayerId)) return false;
		if (!refStream.WriteInt(dwResult)) return false;
		return true;
	}
};

//-----------------------------------------------------------------------
struct stCHAT_SEND_CHAT_MANAGER_INFO
{
	unsigned int m_dwChatPort;
	unsigned int m_dwWebSocketChatPort;
	unsigned int m_dwChatServerPort;
	std::string m_szChatIp;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(m_dwChatPort)) return false;
		if (!refStream.WriteInt(m_dwWebSocketChatPort)) return false;
		if (!refStream.WriteInt(m_dwChatServerPort)) return false;
		if (!refStream.WriteString(m_szChatIp)) return false;
		return true;
	}
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(m_dwChatPort)) return false;
		if (!refStream.ReadInt(m_dwWebSocketChatPort)) return false;
		if (!refStream.ReadInt(m_dwChatServerPort)) return false;
		if (!refStream.ReadString(m_szChatIp)) return false;
		return true;
	}
};

struct stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN
{
	std::string szPlayerId;
	std::string szServerId;
	std::string szSign;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteString(szPlayerId)) return false;
		if (!refStream.WriteString(szServerId)) return false;
		if (!refStream.WriteString(szSign)) return false;
		return true;
	}
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szPlayerId)) return false;
		if (!refStream.ReadString(szServerId)) return false;
		if (!refStream.ReadString(szSign)) return false;
		return true;
	}
};

struct stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN_GM : public stCHAT_SEND_CHAT_MANAGER_LOGIN_SIGN
{

};

struct stCHAT_MANAGER_NOTIFY_CHAT_INFO
{
	struct stRemoteChatInfo
	{
		unsigned int dwIp;
		unsigned int dwPort;
		unsigned int dwHashIndex;
	};
	unsigned int dwHashIndex;
	std::vector<stRemoteChatInfo> vecRemoteInfo;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwHashIndex)) return false;
		if (!refStream.WriteInt((unsigned int)vecRemoteInfo.size())) return false;
		for (std::vector<stRemoteChatInfo>::iterator it = vecRemoteInfo.begin();
			it != vecRemoteInfo.end(); ++it)
		{
			if (!refStream.WriteInt(it->dwIp)) return false;
			if (!refStream.WriteInt(it->dwPort)) return false;
			if (!refStream.WriteInt(it->dwHashIndex)) return false;
		}
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		vecRemoteInfo.clear();
		if (!refStream.ReadInt(dwHashIndex)) return false;
		unsigned int dwLen = 0;
		if (!refStream.ReadInt(dwLen)) return false;
		for (unsigned int i = 0; i < dwLen; ++i)
		{
			stRemoteChatInfo oRemoteChatInfo;
			if (!refStream.ReadInt(oRemoteChatInfo.dwIp)) return false;
			if (!refStream.ReadInt(oRemoteChatInfo.dwPort)) return false;
			if (!refStream.ReadInt(oRemoteChatInfo.dwHashIndex)) return false;
			vecRemoteInfo.push_back(oRemoteChatInfo);
		}
		return true;
	}
};

struct stCHAT_MANAGER_NOTIFY_CHAT_BROADCAST
{
	Protocol::EChatType eChatType;
	std::string szContent;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt((unsigned int&)eChatType)) return false;
		if (!refStream.WriteString(szContent)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt((unsigned int&)eChatType)) return false;
		if (!refStream.ReadString(szContent)) return false;
		return true;
	}
};

struct stCHAT_SEND_CHAT_PRIVATE_CHAT
{
	stCHAT_SEND_CHAT_PRIVATE_CHAT() { memset(szSenderId, 0, IDLENTH); memset(szRecverId, 0, IDLENTH); eChatType = Protocol::ECT_NONE; dwTimeStamp = 0; }
	unsigned int dwTimeStamp;
	char szSenderId[IDLENTH];
	char szRecverId[IDLENTH];
	Protocol::EChatType eChatType;
	std::string szContent;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwTimeStamp)) return false;
		if (!refStream.WriteString(szSenderId)) return false;
		if (!refStream.WriteString(szRecverId)) return false;
		if (!refStream.WriteInt((unsigned int&)eChatType)) return false;
		if (!refStream.WriteString(szContent)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwTimeStamp)) return false;
		if (!refStream.ReadString(szSenderId, IDLENTH)) return false;
		if (!refStream.ReadString(szRecverId, IDLENTH)) return false;
		if (!refStream.ReadInt((unsigned int&)eChatType)) return false;
		if (!refStream.ReadString(szContent)) return false;
		return true;
	}
};

struct stCHAT_MANAGER_NOTIFY_CHAT_LOGIN
{
	std::string szPlayerId;
	std::string szServerId;
	
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteString(szPlayerId)) return false;
		if (!refStream.WriteString(szServerId)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szPlayerId)) return false;
		if (!refStream.ReadString(szServerId)) return false;
		return true;
	}
};

struct stCHAT_MANAGER_NOTIFY_CHAT_LOGIN_GM : public stCHAT_MANAGER_NOTIFY_CHAT_LOGIN
{

};

//----------------------------------------------------------------------
struct stPLAYER_REQUEST_CHAT_LOGIN
{
	stPLAYER_REQUEST_CHAT_LOGIN() { memset(szId, 0, IDLENTH); memset(szSign, 0, 128); }
	char szId[IDLENTH];
	char szSign[128];

	bool Read(CNetStream& refStream)
	{
		if(!refStream.ReadString(szId, IDLENTH)) return false;
		if (!refStream.ReadString(szSign, 128)) return false;
		return true;
	}
};

struct stPLAYER_REQUEST_PRIVATE_CHAT
{
	stPLAYER_REQUEST_PRIVATE_CHAT() { memset(szRecverId, 0, IDLENTH); eChatType = Protocol::ECT_NONE; }
	char szRecverId[IDLENTH];
	Protocol::EChatType eChatType;
	std::string szContent;

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szRecverId, IDLENTH)) return false;
		if (!refStream.ReadInt((unsigned int&)eChatType)) return false;
		if (!refStream.ReadString(szContent)) return false;
		return true;
	}
};

struct stPLAYER_REQUEST_CREATE_CHAT_GROUP
{
	bool Write(CNetStream& refStream)
	{
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		return true;
	}
};

struct stPLAYER_REQUEST_CHAT_GROUP_CHAT
{
	unsigned int dwGroupId;
	Protocol::EChatType eChatType;
	std::string szContent;

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadInt((unsigned int&)eChatType)) return false;
		if (!refStream.ReadString(szContent)) return false;
		return true;
	}
};

struct stPLAYER_REQUEST_INVITE_ENTER_GROUP_CHAT
{
	unsigned int dwGroupId;
	std::string szPlayerId;
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadString(szPlayerId)) return false;
		return true;
	}
};

struct stPLAYER_REQUEST_LEAVE_GROUP_CHAT
{
	unsigned int dwGroupId;
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		return true;
	}
};

//---------------------------------------------------------------
struct stCHAT_ACK_PLAYER_LOGIN
{
	unsigned int dwResult;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwResult)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwResult)) return false;
		return true;
	}
};

struct stCHAT_SEND_PLAYER_PRIVATE_CHAT : public stCHAT_SEND_CHAT_PRIVATE_CHAT
{
};

struct stCHAT_ACK_PLAYER_CREATE_CHAT_GROUP
{
	unsigned int dwGroupId;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		return true;
	}
};

struct stCHAT_ACK_PLAYER_LEAVE_GROUP_CHAT
{
	unsigned int dwGroupId;
	unsigned int dwResult;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwGroupId)) return false;
		if (!refStream.WriteInt(dwResult)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwGroupId)) return false;
		if (!refStream.ReadInt(dwResult)) return false;
		return true;
	}
};

//----------------------------------------------------------------------
struct stGAME_REQUEST_CHAT_MANAGER_LOGIN
{
	stGAME_REQUEST_CHAT_MANAGER_LOGIN() {}
	std::string szId;

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szId)) return false;
		return true;
	}
};

struct stGAME_REQUEST_CHAT_MANAGER_PLAYER_LOGIN
{
	std::string szPlayerId;

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szPlayerId)) return false;
		return true;
	}
};

//----------------------------------------------------------------------
struct stCHAT_MANAGER_ACK_GAME_LOGIN
{
	unsigned int dwResult;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(dwResult)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(dwResult)) return false;
		return true;
	}
};

struct stCHAT_MANAGER_ACK_GAME_PLAYER_LOGIN
{
	std::string szPlayerId;
	std::string szListenIp;
	std::string szSign;
	unsigned int dwTcpPort;
	unsigned int dwWebSocketPort;

	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteString(szPlayerId)) return false;
		if (!refStream.WriteString(szListenIp)) return false;
		if (!refStream.WriteString(szSign)) return false;
		if (!refStream.WriteInt(dwTcpPort)) return false;
		if (!refStream.WriteInt(dwWebSocketPort)) return false;
		return true;
	}

	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadString(szPlayerId)) return false;
		if (!refStream.ReadString(szListenIp)) return false;
		if (!refStream.ReadString(szSign)) return false;
		if (!refStream.ReadInt(dwTcpPort)) return false;
		if (!refStream.ReadInt(dwWebSocketPort)) return false;
		return true;
	}
};




//----------------------------------------------------------------------
class DBChatQuery : public IQuery
{
public:
	DBChatQuery(stCHAT_SEND_CHAT_PRIVATE_CHAT& refCHAT_SEND_CHAT_PRIVATE_CHAT, bool bReaded)
	{
		static char szTemp[2048 * 3 + 1] = { 0 };
		static char szContentEacape[2048 * 3 + 1] = { 0 };
		memset(szTemp, 0, 2048);
		memset(szContentEacape, 0, 2048 * 3 + 1);
		m_bReader = bReaded ? 1 : 0;
		mysql_escape_string(szContentEacape, refCHAT_SEND_CHAT_PRIVATE_CHAT.szContent.c_str(), refCHAT_SEND_CHAT_PRIVATE_CHAT.szContent.size());
		sprintf(szTemp, "INSERT INTO private_chat (`sender_id`, `recver_id`, `chat_type`, `content`, `send_time`, `readed`) "
			"VALUES('%s', '%s', %u, '%s', %u, %u)", refCHAT_SEND_CHAT_PRIVATE_CHAT.szSenderId,
			refCHAT_SEND_CHAT_PRIVATE_CHAT.szRecverId, (int)refCHAT_SEND_CHAT_PRIVATE_CHAT.eChatType,
			szContentEacape, refCHAT_SEND_CHAT_PRIVATE_CHAT.dwTimeStamp, m_bReader);
		m_strQuery = szTemp;
	}
	virtual ~DBChatQuery() {}

	virtual INT32 GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		if (poDBConnection->Query(m_strQuery.c_str()) == FXDB_HAS_RESULT)
		{
			LogExe(LogLv_Critical, "error");
		}
	}

	virtual void OnResult(void)
	{}

	virtual void Release(void)
	{
		delete this;
	}
private:
	std::string m_strQuery;
	unsigned int m_bReader;
};









#endif // !__ChatDefine_H__