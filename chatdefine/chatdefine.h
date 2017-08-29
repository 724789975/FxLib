#ifndef __ChatDefine_H__
#define __ChatDefine_H__

namespace ChatConstant
{
	static const unsigned int g_dwChatServerNum = 4;
	static const unsigned int g_dwHashGen = 256;
}

namespace Protocol
{
	enum EChatProtocol
	{
		//chat<--->chat 10001 11000
		CHAT_TO_CHAT_BEGIN = 10001,
		CHAT_TO_CHAT_END = 11000,

		//chat --->chatmanager 20001 25000
		CHAT_TO_CHAT_MANAGER_BEGIN = 20001,
		CHAT_SEND_CHAT_MANAGER_INFO,
		CHAT_TO_CHAT_MANAGER_END = 25000,

		//chatmanager --->chat 25001 29999
		CHAT_MANAGER_TO_CHAT_BEGIN = 25001,
		CHAT_MANAGER_NOTIFY_CHAT_INFO,
		CHAT_MANAGER_TO_CHAT_END = 29999,
	};
}

#endif // !__ChatDefine_H__