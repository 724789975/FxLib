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


#endif // !__GameDefine_H__
