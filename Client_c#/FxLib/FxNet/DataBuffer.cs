using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;

namespace FxNet
{
	public class DataBuffer
	{
		public DataBuffer()
		{
			m_dwUsedLength = 0;
			m_obj = new object();
			m_pData = new byte[MAX_DATA_LENGTH];
		}

		public byte[] GetData() { return m_pData; }

		public UInt32 GetFreeLength() { return MAX_DATA_LENGTH - m_dwUsedLength; }

		public bool PushData(byte[] byteData, UInt32 dwDataLen)
		{
			if (MAX_DATA_LENGTH - m_dwUsedLength < dwDataLen)
			{
				return false;
			}
			lock (m_obj)
			{
				Array.Copy(byteData, 0, m_pData, (int)m_dwUsedLength, dwDataLen);
				m_dwUsedLength += dwDataLen;
				return true;
			}
		}

		/// <summary>
		/// 主线程调用
		/// </summary>
		/// <param name="byteData"></param>
		/// <param name="dwLen"></param>
		/// <returns></returns>
		public bool PopData(byte[] byteData, UInt32 dwLen)
		{
			if (m_dwUsedLength < dwLen)
			{
				return false;
			}
			lock (m_obj)
			{
				Array.Copy(m_pData, byteData, dwLen);
				m_dwUsedLength -= dwLen;
				Array.Copy(m_pData, dwLen, m_pData, 0, m_dwUsedLength);
				return true;
			}
		}

		public const UInt32 MAX_DATA_LENGTH = 64 * 1024;
		byte[] m_pData;

		UInt32 m_dwUsedLength;
		object m_obj;
	}


	public class CNetStream
	{
		public enum ENetStreamType
		{
			ENetStreamType_None,
			ENetStreamType_Read,
			ENetStreamType_Write,
		}
		CNetStream(ENetStreamType eType, byte[] pData, UInt32 dwLen)
		{
			m_eType = eType;
			m_pData = pData;
			m_dwLen = dwLen;

			switch (eType)
			{
				case ENetStreamType.ENetStreamType_Read:
					{
						MemoryStream ms = new MemoryStream(pData, 0, (int)dwLen);
						m_pReader = new BinaryReader(ms);
					}
					break;
				case ENetStreamType.ENetStreamType_Write:
					{
						MemoryStream ms = new MemoryStream(pData, 0, (int)dwLen);
						m_pWrite = new BinaryWriter(ms);
					}
					break;
				default:
					break;
			}

		}
		CNetStream(byte[] pData, UInt32 dwLen)
		{
			m_eType = ENetStreamType.ENetStreamType_Read;
			m_pData = pData;
			m_dwLen = dwLen;
			MemoryStream ms = new MemoryStream(pData, 0, (int)dwLen);
			m_pReader = new BinaryReader(ms);
		}
		~CNetStream() { }

		UInt32 GetDataLength() { return m_dwLen; }

		bool ReadData(ref byte[] pData, UInt32 dwLen)
		{
			//assert(m_eType == ENetStreamType_Read);
			//if (dwLen > m_dwLen)
			//{
			//	return NULL;
			//}
			//byte[] pTemp = m_pData;
			//m_pData += dwLen;
			//m_dwLen -= dwLen;
			//return pTemp;
			return true;
		}

		bool ReadByte(ref byte cData)
		{
			//byte[] pData = ReadData(sizeof(cData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&cData, pData, sizeof(cData));
			return true;
		}

		bool ReadShort(ref Int16 wData)
		{
			//byte[] pData = ReadData(sizeof(wData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&wData, pData, sizeof(wData));
			//wData = ntohs(wData);
			return true;
		}

		bool ReadShort(ref UInt16 wData)
		{
			//byte[] pData = ReadData(sizeof(wData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&wData, pData, sizeof(wData));
			//wData = ntohs(wData);
			return true;
		}

		bool ReadInt(ref Int32 dwData)
		{
			//byte[] pData = ReadData(sizeof(dwData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&dwData, pData, sizeof(dwData));
			//dwData = ntohl(dwData);
			return true;
		}

		bool ReadInt(ref UInt32 dwData)
		{
			//byte[] pData = ReadData(sizeof(dwData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&dwData, pData, sizeof(dwData));
			//dwData = ntohl(dwData);
			return true;
		}

		bool ReadInt64(ref Int64 llData)
		{
			//byte[] pData = ReadData(sizeof(llData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&llData, pData, sizeof(llData));
			//union ByteOrder
			//         {
			//	unsigned short wByte;
			//	unsigned char ucByte[2];
			//};
			//static const short wByte = 0x0102;
			//static const ByteOrder&oByteOrder = (ByteOrder &)wByte;
			//if (oByteOrder.ucByte[0] != 0x01)
			//{
			//	// 小端
			//	llData = (long long)htonl((int)(llData >> 32)) | ((long long)htonl((int)llData) << 32);
			//}
			return true;
		}

		bool ReadInt64(ref UInt64 ullData)
		{
			//byte[] pData = ReadData(sizeof(ullData));
			//if (!pData)
			//{
			//	return false;
			//}
			//memcpy(&ullData, pData, sizeof(ullData));
			//union ByteOrder
			//         {
			//	unsigned short wByte;
			//	unsigned char ucByte[2];
			//};
			//static const short wByte = 0x0102;
			//static const ByteOrder&oByteOrder = (ByteOrder &)wByte;
			//if (oByteOrder.ucByte[0] != 0x01)
			//{
			//	// 小端
			//	ullData = (unsigned long long)htonl((int)(ullData >> 32)) | ((unsigned long long)htonl((int)ullData) << 32);
			//}
			return true;
		}

		bool ReadFloat(float fData)
		{
			//int dwFloat = 0;
			//if (!ReadInt(dwFloat))
			//{
			//	return false;
			//}
			//fData = dwFloat / 256.0f;
			return true;
		}

		byte[] GetData(UInt32 dwLen)
		{
			//assert(m_eType == ENetStreamType_Read);
			//if (dwLen <= m_dwLen)
			//{
			//	return m_pData;
			//}
			return null;
		}

		bool WriteData(byte[] pData, UInt32 dwLen)
		{
			//assert(m_eType == ENetStreamType_Write);
			//if (dwLen > m_dwLen)
			//{
			//	return false;
			//}
			//memcpy(m_pData, pData, dwLen);
			//m_pData += dwLen;
			//m_dwLen -= dwLen;
			return true;
		}

		bool WriteByte(byte cData)
		{
			//return WriteData(cData, sizeof(byte));
			return true;
		}

		bool WriteShort(Int16 wData)
		{
			wData = IPAddress.HostToNetworkOrder(wData);
			m_pWrite.Write(wData);
			return true;
		}

		bool WriteShort(UInt16 wData)
		{
			return WriteShort((Int16)wData);
		}

		bool WriteInt(Int32 dwData)
		{
			dwData = IPAddress.HostToNetworkOrder(dwData);
			m_pWrite.Write(dwData);
			return true;
		}

		bool WriteInt(UInt32 dwData)
		{
			return WriteInt((Int32)dwData);
		}

		bool WriteInt64(Int64 llData)
		{
			llData = IPAddress.HostToNetworkOrder(llData);
			m_pWrite.Write(llData);
			return true;
		}

		bool WriteInt64(UInt64 ullData)
		{
			return WriteInt64((Int64)ullData);
		}

		bool WriteFloat(float fData)
		{
			Int32 nData = (Int32)(fData * 256);
			return WriteInt(nData);
		}

		ENetStreamType m_eType;
		byte[] m_pData;
		UInt32 m_dwLen;
		BinaryReader m_pReader;
		BinaryWriter m_pWrite;
	};
}
