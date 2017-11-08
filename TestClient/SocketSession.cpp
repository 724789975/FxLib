#include "SocketSession.h"
#include <stdio.h>
#include <string.h>
#include "netstream.h"
#include <stdlib.h>

CSocketSession::CSocketSession()
{
}

CSocketSession::~CSocketSession()
{
}

void CSocketSession::OnConnect(void)
{
	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
	//char szMsg[1024] = {0};
	//CNetStream oStream(ENetStreamType_Write, szMsg, 1024);
	//oStream.WriteInt(40001);
	//oStream.WriteString("asdg");
	////oStream.WriteString("sign");
	//Send(szMsg, 1024 - oStream.GetDataLength());
}

void CSocketSession::OnClose(void)
{
	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
	Reconnect();
}

void CSocketSession::OnError(UINT32 dwErrorNo)
{
	LogFun(LT_Screen | LT_File, LogLv_Debug, "ip : %s, port : %d, error no : %d\n", GetRemoteIPStr(), GetRemotePort(), dwErrorNo);
}

void CSocketSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), pBuf);
	//CNetStream oStream1(pBuf, 1024);
	//UINT32 dwProtocol;
	//oStream1.ReadInt(dwProtocol);
	//UINT32 dwResult;
	//oStream1.ReadInt(dwResult);
	////char szMsg[1024] = "";
	////sprintf(szMsg, "%d", atoi(pBuf) + 1);
	////Send(szMsg, 1024);

	//char szMsg[1024] = { 0 };
	//CNetStream oStream(ENetStreamType_Write, szMsg, 1024);
	//oStream.WriteInt(30003);
	//oStream.WriteString("asdg1");
	//oStream.WriteInt(1);
	//oStream.WriteString("ggggggggggasdg1");
	//Send(szMsg, 1024 - oStream.GetDataLength());
}

void CSocketSession::Release(void)
{
	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

FxSession*	CSessionFactory::CreateSession()
{
	FxSession* pSession = new CBinarySocketSession();
	return pSession;
}

BinaryDataHeader::BinaryDataHeader()
{

}

BinaryDataHeader::~BinaryDataHeader()
{

}

void* BinaryDataHeader::GetPkgHeader()
{
	return (void*)m_dataBuffer;
}

void* BinaryDataHeader::BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen)
{
	dwHeaderLen = sizeof(m_dataBuffer);
	//*((UINT32*)m_dataBuffer) = htonl(dwDataLen);
	CNetStream oNetStream(ENetStreamType_Write, m_dataBuffer, sizeof(m_dataBuffer));
	oNetStream.WriteInt(dwDataLen);
	oNetStream.WriteInt(s_dwMagic);
	return (void*)m_dataBuffer;
}

bool BinaryDataHeader::BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset)
{
	if (dwLen + dwOffset > GetHeaderLength())
	{
		return false;
	}

	memcpy(m_dataBuffer + dwOffset, pBuff, dwLen);
	return true;
}

int BinaryDataHeader::__CheckPkgHeader(const char* pBuf)
{
	CNetStream oHeaderStream(m_dataBuffer, sizeof(m_dataBuffer));
	CNetStream oRecvStream(pBuf, sizeof(m_dataBuffer));

	UINT32 dwHeaderLength = 0;
	UINT32 dwBufferLength = 0;
	oHeaderStream.ReadInt(dwHeaderLength);
	oRecvStream.ReadInt(dwBufferLength);

	UINT32 dwHeaderMagic = 0;
	UINT32 dwBufferMagic = 0;
	oHeaderStream.ReadInt(dwHeaderMagic);
	oRecvStream.ReadInt(dwBufferMagic);

	if (dwHeaderLength != dwBufferLength)
	{
		return -1;
	}

	if (s_dwMagic != dwBufferMagic)
	{
		return -1;
	}

	return (GetHeaderLength() + dwHeaderLength);
}
