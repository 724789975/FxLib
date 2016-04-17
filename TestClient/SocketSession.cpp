#include "SocketSession.h"
#include <stdio.h>
#include <string.h>
#include "netstream.h"

CSocketSession::CSocketSession()
{
}

CSocketSession::~CSocketSession()
{
}

void CSocketSession::OnConnect(void)
{
	LogScreen("ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

void CSocketSession::OnClose(void)
{
	LogScreen("ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
//	Reconnect();
}

void CSocketSession::OnError(UINT32 dwErrorNo)
{
	LogScreen("ip : %s, port : %d, error no : %d\n", GetRemoteIPStr(), GetRemotePort(), dwErrorNo);
}

void CSocketSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	LogScreen("ip : %s, port : %d, recv %s\n", GetRemoteIPStr(), GetRemotePort(), pBuf);
}

void CSocketSession::Release(void)
{
	LogScreen("ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

FxSession*	CSessionFactory::CreateSession()
{
	FxSession* pSession = new CSocketSession();
	pSession->SetDataHeader(oDataHeaderFactory.CreateDataHeader());
	return pSession;
}

DataHeader::DataHeader()
{

}

DataHeader::~DataHeader()
{

}

void* DataHeader::GetPkgHeader()
{
	return (void*)m_dataBuffer;
}

void* DataHeader::BuildSendPkgHeader(UINT32 dwDataLen)
{
	//*((UINT32*)m_dataBuffer) = htonl(dwDataLen);
	CNetStream oNetStream(ENetStreamType_Write, m_dataBuffer, sizeof(m_dataBuffer));
	oNetStream.WriteInt(dwDataLen);
	return (void*)m_dataBuffer;
}

bool DataHeader::BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset)
{
	if (dwLen + dwOffset > GetHeaderLength())
	{
		return false;
	}

	memcpy(m_dataBuffer + dwOffset, pBuff, dwLen);
	return true;
}

int DataHeader::__CheckPkgHeader(const char* pBuf)
{
	CNetStream oHeaderStream(m_dataBuffer, sizeof(m_dataBuffer));
	CNetStream oRecvStream(pBuf, sizeof(m_dataBuffer));

	UINT32 dwHeaderLength = 0;
	UINT32 dwBufferLength = 0;
	oHeaderStream.ReadInt(dwHeaderLength);
	oRecvStream.ReadInt(dwBufferLength);

	if (dwHeaderLength != dwBufferLength)
	{
		return -1;
	}

	return (GetHeaderLength() + dwHeaderLength);
}
