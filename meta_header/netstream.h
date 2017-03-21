#ifndef __NetStream_H__
#define __NetStream_H__

#ifdef WIN32
#include<Winsock2.h>
#else
#include <arpa/inet.h>
#endif // WIN32

#include <string.h>


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
			char* pTemp = m_pData;
			m_pData += dwLen;
			m_dwLen -= dwLen;
			return pTemp;
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

	bool ReadInt64(long long& llData)
	{
		char* pData = ReadData(sizeof(llData));
		if (pData)
		{
			memcpy(&llData, pData, sizeof(llData));
			union
			{
				unsigned short wByte;
				unsigned char ucByte[2];
			} oByteOrder;
			oByteOrder.wByte = 0x0102;
			if (oByteOrder.ucByte[0] != 0x01)
			{
				// 小端
				llData = (long long)htonl((int)(llData >> 32)) | ((long long)htonl((int)llData) << 32);
			}
			return true;
		}
		return false;
	}

	bool ReadInt64(unsigned long long& ullData)
	{
		char* pData = ReadData(sizeof(ullData));
		if (pData)
		{
			memcpy(&ullData, pData, sizeof(ullData));
			union
			{
				unsigned short wByte;
				unsigned char ucByte[2];
			} oByteOrder;
			oByteOrder.wByte = 0x0102;
			if (oByteOrder.ucByte[0] != 0x01)
			{
				// 小端
				ullData = (unsigned long long)htonl((int)(ullData >> 32)) | ((unsigned long long)htonl((int)ullData) << 32);
			}
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

	//  //
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

	bool WriteData(const char* pData, unsigned int dwLen)
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

	bool WriteInt64(long long& llData)
	{
		union
		{
			unsigned short wByte;
			unsigned char ucByte[2];
		} oByteOrder;
		oByteOrder.wByte = 0x0102;
		if (oByteOrder.ucByte[0] != 0x01)
		{
			// 小端
			llData = (long long)htonl((int)(llData >> 32)) | ((long long)htonl((int)llData) << 32);
		}
		return WriteData((char*)(&llData), sizeof(llData));
	}

	bool WriteInt64(unsigned long long& ullData)
	{
		union
		{
			unsigned short wByte;
			unsigned char ucByte[2];
		} oByteOrder;
		oByteOrder.wByte = 0x0102;
		if (oByteOrder.ucByte[0] != 0x01)
		{
			// 小端
			ullData = (unsigned long long)htonl((int)(ullData >> 32)) | ((unsigned long long)htonl((int)ullData) << 32);
		}
		return WriteData((char*)(&ullData), sizeof(ullData));
	}

	bool WriteFloat(float fData)
	{
		int nData = (int)(fData * 256);
		return WriteInt(nData);
	}

private:
	ENetStreamType m_eType;
	char* m_pData;
	unsigned int m_dwLen;
};

#endif	//	__NetStream_H__
