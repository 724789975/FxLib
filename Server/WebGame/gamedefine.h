#ifndef __GameDefine_H__
#define __GameDefine_H__

#include <vector>
#include "netstream.h"
#include "fxdb.h"

#define IDLENTH 64

namespace Protocol
{
	enum EGameProtocol
	{
		//game->gamemanager
		GAME_TO_GAME_MANAGER_BEGIN = 10001,
		GAME_NOTIFY_GAME_MANAGER_INFO = 10002,
		GAME_TO_GAME_MANAGER_END = 11000,
		//gamemanager->game
		GAME_MANAGER_TO_GAME_BEGIN = 20001,
		GAME_MANAGER_ACK_GAME_INFO_RESULT = 20002,
		GAME_MANAGER_TO_GAME_END = 21000,

		//player->gamemanager
		PLAYER_TO_GAME_MANAGER_BEGIN = 30001,
		PLAYER_REQUEST_GAME_MANAGER_INFO = 30002,
		PLAYER_TO_GAME_MANAGER_END = 31000,
		//gamemanager->player
		GAME_MANAGER_TO_PLAYER_BEGIN = 40001,
		GAME_MANAGER_ACK_PLAYER_INFO_RESULT = 40002,
		GAME_MANAGER_TO_PLAYER_END = 41000,
	};
}

//----------------------------------------------------------------------
struct stGAME_NOTIFY_GAME_MANAGER_INFO
{
	unsigned short wPort;
	bool Write(CNetStream& refStream)
	{
		return refStream.WriteShort(wPort);
	}
	bool Read(CNetStream& refStream)
	{
		return refStream.WriteShort(wPort);
	}
};
//----------------------------------------------------------------------
struct stGAME_MANAGER_ACK_GAME_INFO_RESULT
{
	unsigned int dwResult;
	bool Write(CNetStream& refStream)
	{
		return refStream.WriteInt(dwResult);
	}
	bool Read(CNetStream& refStream)
	{
		return refStream.ReadInt(dwResult);
	}
};
//----------------------------------------------------------------------
struct stPLAYER_REQUEST_GAME_MANAGER_INFO
{
	bool Write(CNetStream& refStream)
	{
		return false;
	}
	bool Read(CNetStream& refStream)
	{
		return false;
	}
};
//----------------------------------------------------------------------
struct stGAME_MANAGER_ACK_PLAYER_INFO_RESULT
{
	bool Write(CNetStream& refStream)
	{
		return false;
	}
	bool Read(CNetStream& refStream)
	{
		return false;
	}
};
//----------------------------------------------------------------------


#endif // !__GameDefine_H__
