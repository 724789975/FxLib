#ifndef __ChatDefine_H__
#define __ChatDefine_H__

#include <vector>
#include "../meta_header/netstream.h"

#define IDLENTH 64

namespace ChatConstant
{
	static const unsigned int g_dwChatServerNum = 3;
	static const unsigned int g_dwHashGen = 256;
}

namespace Protocol
{
	enum EChatProtocol
	{
		//chat<--->chat 10001 11000
		CHAT_TO_CHAT_BEGIN = 10001,
		CHAT_TO_CHAT_HASH_INDEX = 10002,
		CHAT_TO_CHAT_END = 11000,

		//chat --->chatmanager 20001 25000
		CHAT_TO_CHAT_MANAGER_BEGIN = 20001,
		CHAT_SEND_CHAT_MANAGER_INFO,
		CHAT_TO_CHAT_MANAGER_END = 25000,

		//chatmanager --->chat 25001 29999
		CHAT_MANAGER_TO_CHAT_BEGIN = 25001,
		CHAT_MANAGER_NOTIFY_CHAT_INFO,
		CHAT_MANAGER_TO_CHAT_END = 29999,

		//player<--->chat 30000 39999
		PLAYER_CHAT_BEGIN = 30000,
		PLAYER_LOGIN,
		PLAYER_CHAT_END = 39999,
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

struct stCHAT_SEND_CHAT_MANAGER_INFO
{
	unsigned int m_dwChatPort;
	unsigned int m_dwChatServerPort;
	std::string m_szChatIp;
	bool Write(CNetStream& refStream)
	{
		if (!refStream.WriteInt(m_dwChatPort)) return false;
		if (!refStream.WriteInt(m_dwChatServerPort)) return false;
		if (!refStream.WriteString(m_szChatIp)) return false;
		return true;
	}
	bool Read(CNetStream& refStream)
	{
		if (!refStream.ReadInt(m_dwChatPort)) return false;
		if (!refStream.ReadInt(m_dwChatServerPort)) return false;
		if (!refStream.ReadString(m_szChatIp)) return false;
		return true;
	}
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
		if (!refStream.WriteInt(vecRemoteInfo.size())) return false;
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

//----------------------------------------------------------------------
struct stPLAYER_LOGIN
{
	stPLAYER_LOGIN() { memset(szId, 0, IDLENTH); memset(szSign, 0, 128); }
	char szId[IDLENTH];
	char szSign[128];

	bool Read(CNetStream& refStream)
	{
		if(!refStream.ReadString(szId, IDLENTH)) return false;
		if (!refStream.ReadString(szSign, 128)) return false;
		return true;
	}
};










#endif // !__ChatDefine_H__