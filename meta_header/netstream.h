#ifndef __NetStream_H__
#define __NetStream_H__

#ifdef WIN32
#include<Winsock2.h>
#else
#include <arpa/inet.h>
#endif // WIN32


enum ENetStreamType
{
	ENetStreamType_None,
	ENetStreamType_Read,
	ENetStreamType_Write,
};

class CNetStream
{
public:
	CNetStream(ENetStreamType eType, char* pData, unsigned int dwLen)
		:m_eType(eType)
		,m_pData(pData)
		,m_dwLen(dwLen)
	{
	}

	CNetStream(const char* pData, unsigned int dwLen)
		:m_eType(ENetStreamType_Read)
		,m_pData(const_cast<char*>(pData))
		,m_dwLen(dwLen)
	{
	}
	~CNetStream() {}

	unsigned int GetDataLength() { return m_dwLen; }

	char* ReadData(unsigned int dwLen)
	{
		assert(m_eType == ENetStreamType_Read);
		if (dwLen <= m_dwLen)
		{
			m_pData += dwLen;
			m_dwLen -= dwLen;
			return m_pData;
		}
		return NULL;
	}

	bool ReadByte(char& cData)
	{
		char* pData = ReadData(sizeof(cData));
		if (pData)
		{
			memcpy(&cData, pData, sizeof(cData));
			return true;
		}
		return false;
	}

	bool ReadByte(unsigned char& cData)
	{
		char* pData = ReadData(sizeof(cData));
		if (pData)
		{
			memcpy(&cData, pData, sizeof(cData));
			return true;
		}
		return false;
	}

	bool ReadShort(short& wData)
	{
		char* pData = ReadData(sizeof(wData));
		if (pData)
		{
			memcpy(&wData, pData, sizeof(wData));
			wData = ntohs(wData);
			return true;
		}
		return false;
	}

	bool ReadShort(unsigned short& wData)
	{
		char* pData = ReadData(sizeof(wData));
		if (pData)
		{
			memcpy(&wData, pData, sizeof(wData));
			wData = ntohs(wData);
			return true;
		}
		return false;
	}

	bool ReadInt(int& dwData)
	{
		char* pData = ReadData(sizeof(dwData));
		if (pData)
		{
			memcpy(&dwData, pData, sizeof(dwData));
			dwData = ntohl(dwData);
			return true;
		}
		return false;
	}

	bool ReadInt(unsigned int& dwData)
	{
		char* pData = ReadData(sizeof(dwData));
		if (pData)
		{
			memcpy(&dwData, pData, sizeof(dwData));
			dwData = ntohl(dwData);
			return true;
		}
		return false;
	}

	bool ReadFloat(float& fData)
	{
		int dwFloat = 0;
		if (ReadInt(dwFloat))
		{
			fData = dwFloat / 256.0f;
			return true;
		}
		return false;
	}

	//只能获取开头的那段数据 用来获取信息头//
	const char* GetData(unsigned int dwLen)
	{
		assert(m_eType == ENetStreamType_Read);
		if (dwLen <= m_dwLen)
		{
			return m_pData;
		}
		return NULL;
	}

	bool WriteData(char* pData, unsigned int dwLen)
	{
		assert(m_eType == ENetStreamType_Write);
		if (dwLen <= m_dwLen)
		{
			memcpy(m_pData, pData, dwLen);
			m_pData += dwLen;
			m_dwLen -= dwLen;
			return true;
		}
		return false;
	}

	bool WriteByte(char cData)
	{
		return WriteData(&cData, sizeof(cData));
	}

	bool WriteByte(unsigned char cData)
	{
		return WriteData((char*)(&cData), sizeof(cData));
	}

	bool WriteShort(short wData)
	{
		wData = htons(wData);
		return WriteData((char*)(&wData), sizeof(wData));
	}

	bool WriteShort(unsigned short wData)
	{
		wData = htons(wData);
		return WriteData((char*)(&wData), sizeof(wData));
	}

	bool WriteInt(int dwData)
	{
		dwData = htonl(dwData);
		return WriteData((char*)(&dwData), sizeof(dwData));
	}

	bool WriteInt(unsigned int dwData)
	{
		dwData = htonl(dwData);
		return WriteData((char*)(&dwData), sizeof(dwData));
	}

	bool WriteFloat(float fData)
	{
		int nData = fData * 256;
		return WriteInt(nData);
	}

private:
	ENetStreamType m_eType;
	char* m_pData;
	unsigned int m_dwLen;
};

#endif	//	__NetStream_H__