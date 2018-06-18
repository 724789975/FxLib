#ifndef __GameDefine_H__
#define __GameDefine_H__

#include "netstream.h"

#include "google/protobuf/message.h"

#define IDLENTH 64
#define MAXCLIENTNUM 16
#define MAXSLAVESERVERNUM 20

namespace ProtoUtility
{
	static void MakeProtoSendBuffer(google::protobuf::Message& refMsg, char* & refpBuf, unsigned int& dwLen)
	{
		const static unsigned int dwBuffLen = 64 * 1024;
		static char pBuf[dwBuffLen];
		CNetStream oStream(ENetStreamType_Write, pBuf, dwBuffLen);
		oStream.WriteString(refMsg.GetTypeName());

		std::string szInfo = refMsg.SerializeAsString();
		oStream.WriteData(szInfo.c_str(), szInfo.size());
		refpBuf = pBuf;
		dwLen = dwBuffLen - oStream.GetDataLength();
	}
}

namespace RedisConstant
{
	static const char* szTeamId = "team_id";
	static const char* szPlayerTeamId = "player_team_id";
	static const char* szOnLinePlayer = "online_player";	//在线的玩家 <playerid, serverid>
	static const char* szGameConfig = "game_config";
	static const char* szGameType = "game_type";
	static const char* szPrepareTime = "prepare_time";
	static const char* szGameReadyTime = "game_ready_time";
	static const char* szSuspendTime = "suspend_time";
	static const char* szGameIp = "game_ip";
	static const char* szGamePort = "game_port";
}


#endif // !__GameDefine_H__
