#include "SocketSession.h"
#include <string>
#include <stdio.h>
#include "fxdb.h"
#include "netstream.h"
#include <map>

BinaryDataHeader::BinaryDataHeader()
{
}

BinaryDataHeader::~BinaryDataHeader()
{

}

void* BinaryDataHeader::GetPkgHeader()
{
	return (void*)m_dataRecvBuffer;
}

void* BinaryDataHeader::BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen)
{
	//*((UINT32*)m_dataBuffer) = htonl(dwDataLen);
	dwHeaderLen = sizeof(m_dataSendBuffer);
	CNetStream oNetStream(ENetStreamType_Write, m_dataSendBuffer, sizeof(m_dataSendBuffer));
	oNetStream.WriteInt(dwDataLen);
	oNetStream.WriteInt(s_dwMagic);
	return (void*)m_dataSendBuffer;
}

bool BinaryDataHeader::BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset)
{
	if (dwLen + dwOffset > GetHeaderLength())
	{
		return false;
	}

	memcpy(m_dataRecvBuffer + dwOffset, pBuff, dwLen);
	return true;
}

int BinaryDataHeader::__CheckPkgHeader(const char* pBuf)
{
	CNetStream oHeaderStream(m_dataRecvBuffer, sizeof(m_dataRecvBuffer));
	CNetStream oRecvStream(pBuf, sizeof(m_dataRecvBuffer));

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

WebSocketDataHeader::WebSocketDataHeader()
	:m_dwHeaderLength(0)
{
}

WebSocketDataHeader::~WebSocketDataHeader()
{
}

void * WebSocketDataHeader::GetPkgHeader()
{
	return (void*)m_dataRecvBuffer;
}

void* WebSocketDataHeader::BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen)
{
	dwHeaderLen = 1;
	CNetStream oNetStream(ENetStreamType_Write, m_dataSendBuffer, sizeof(m_dataSendBuffer));
	unsigned char btFinOpCode = 0x82;
	// unsigned char btFin = (btFinOpCode >> 7) & 0xff;
	// unsigned char btOpCode = (btFinOpCode) & 0x0f;
	oNetStream.WriteByte(btFinOpCode);
	if (dwDataLen < 126 && dwDataLen <= 0xFFFF)
	{
		unsigned char btLen = dwDataLen;
		oNetStream.WriteByte(btLen);
		dwHeaderLen += 1;
	}
	else if (dwDataLen >= 126 && dwDataLen <= 0xFFFF)
	{
		unsigned char btLen = 126;
		oNetStream.WriteByte(btLen);
		unsigned short wLen = dwDataLen;
		oNetStream.WriteShort(wLen);
		dwHeaderLen += 3;
	}
	else
	{
		unsigned char btLen = 127;
		oNetStream.WriteByte(btLen);
		unsigned long long qwLen = dwDataLen;
		oNetStream.WriteInt64(qwLen);
		dwHeaderLen += 9;
	}
	return (void*)m_dataSendBuffer;
}

bool WebSocketDataHeader::BuildRecvPkgHeader(char * pBuff, UINT32 dwLen, UINT32 dwOffset)
{
	memcpy(m_dataRecvBuffer + dwOffset, pBuff, sizeof(m_dataRecvBuffer) - dwOffset > dwLen ? dwLen : sizeof(m_dataRecvBuffer) - dwOffset);
	m_dwHeaderLength = 0;
	if (dwLen + dwOffset > 2)
	{
		__CheckPkgHeader(m_dataRecvBuffer);
	}
	return true;
}

int WebSocketDataHeader::__CheckPkgHeader(const char * pBuf)
{
	m_dwHeaderLength = 0;
	CNetStream oHeaderStream(pBuf, sizeof(m_dataRecvBuffer));
	
	unsigned char bt1 = 0, bt2 = 0;
	oHeaderStream.ReadByte(bt1);
	oHeaderStream.ReadByte(bt2);
	m_dwHeaderLength += 2;
	m_btFin = (bt1 >> 7) & 0xff;
	m_btOpCode = (bt1) & 0x0f;
	m_btMask = (bt2 >> 7) & 0xff;
	m_qwPayloadLen = bt2 & 0x7f;

	if (m_qwPayloadLen == 126)
	{
		unsigned short wTemp = 0;
		oHeaderStream.ReadShort(wTemp);
		m_qwPayloadLen = wTemp;
		m_dwHeaderLength += 2;
	}
	else if (m_qwPayloadLen == 127)
	{
		unsigned long long qwTemp = 0;
		oHeaderStream.ReadInt64(qwTemp);
		m_qwPayloadLen = qwTemp;
		m_dwHeaderLength += 8;
	}
	
	if (m_btMask)
	{
		memcpy(m_btMaskingKey, oHeaderStream.ReadData(sizeof(m_btMaskingKey)), sizeof(m_btMaskingKey));
		m_dwHeaderLength += 4;
	}

	return (int)(m_qwPayloadLen + m_dwHeaderLength);
}

int WebSocketDataHeader::ParsePacket(const char * pBuf, UINT32 dwLen)
{
	if (dwLen < 2)
	{
		return 0;
	}

	int nPkgLen = __CheckPkgHeader(m_dataRecvBuffer);

	if (dwLen < m_dwHeaderLength)
	{
		return 0;
	}

	return nPkgLen;
}
