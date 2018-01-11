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
		public UInt32 GetUsedLength() { return m_dwUsedLength; }

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

		public bool PopData(UInt32 dwLen)
		{
			if (m_dwUsedLength < dwLen)
			{
				return false;
			}
			lock (m_obj)
			{
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


	public class NetStream
	{
		public enum ENetStreamType
		{
			ENetStreamType_None,
			ENetStreamType_Read,
			ENetStreamType_Write,
		}
		public NetStream(ENetStreamType eType, byte[] pData, UInt32 dwLen)
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
		public NetStream(byte[] pData, UInt32 dwLen)
		{
			m_eType = ENetStreamType.ENetStreamType_Read;
			m_pData = pData;
			m_dwLen = dwLen;
			MemoryStream ms = new MemoryStream(pData, 0, (int)dwLen);
			m_pReader = new BinaryReader(ms);
		}
		~NetStream() { }

		public bool ReadData(ref byte[] pData, UInt32 dwLen)
		{
			pData = m_pReader.ReadBytes((int)dwLen);
			m_dwLen -= dwLen;
			return true;
		}

		public bool ReadByte(ref byte cData)
		{
			cData = m_pReader.ReadByte();
			m_dwLen -= 1;
			return true;
		}

		public bool ReadShort(ref Int16 wData)
		{
			wData = m_pReader.ReadInt16();
			wData = IPAddress.NetworkToHostOrder(wData);
			m_dwLen -= 2;
			return true;
		}

		public bool ReadShort(ref UInt16 wData)
		{
			wData = m_pReader.ReadUInt16();
			wData = (UInt16)IPAddress.NetworkToHostOrder((Int16)wData);
			m_dwLen -= 2;
			return true;
		}

		public bool ReadInt(ref Int32 dwData)
		{
			dwData = m_pReader.ReadInt32();
			dwData = IPAddress.NetworkToHostOrder(dwData);
			m_dwLen -= 4;
			return true;
		}

		public bool ReadInt(ref UInt32 dwData)
		{
			dwData = m_pReader.ReadUInt32();
			dwData = (UInt32)IPAddress.NetworkToHostOrder((int)dwData);
			m_dwLen -= 4;
			return true;
		}

		public bool ReadInt64(ref Int64 llData)
		{
			llData = m_pReader.ReadInt64();
			llData = IPAddress.NetworkToHostOrder(llData);
			m_dwLen -= 8;
			return true;
		}

		public bool ReadInt64(ref UInt64 ullData)
		{
			ullData = m_pReader.ReadUInt64();
			ullData = (UInt64)IPAddress.NetworkToHostOrder((Int64)ullData);
			m_dwLen -= 8;
			return true;
		}

		public bool ReadFloat(ref float fData)
		{
			int dwFloat = m_pReader.ReadInt32();
			fData = dwFloat / 256.0f;
			return true;
		}

		public bool WriteData(byte[] pData, UInt32 dwLen)
		{
			m_pWrite.Write(pData, 0, (int)dwLen);
			m_dwLen -= dwLen;
			return true;
		}

		public bool WriteByte(byte cData)
		{
			m_pWrite.Write(cData);
			m_dwLen -= 1;
			return true;
		}

		public bool WriteShort(Int16 wData)
		{
			wData = IPAddress.HostToNetworkOrder(wData);
			m_pWrite.Write(wData);
			m_dwLen -= 2;
			return true;
		}

		public bool WriteShort(UInt16 wData)
		{
			return WriteShort((Int16)wData);
		}

		public bool WriteInt(Int32 dwData)
		{
			dwData = IPAddress.HostToNetworkOrder(dwData);
			m_pWrite.Write(dwData);
			m_dwLen -= 4;
			return true;
		}

		public bool WriteInt(UInt32 dwData)
		{
			return WriteInt((Int32)dwData);
		}

		public bool WriteInt64(Int64 llData)
		{
			llData = IPAddress.HostToNetworkOrder(llData);
			m_pWrite.Write(llData);
			m_dwLen -= 8;
			return true;
		}

		public bool WriteInt64(UInt64 ullData)
		{
			return WriteInt64((Int64)ullData);
		}

		public bool WriteFloat(float fData)
		{
			Int32 nData = (Int32)(fData * 256);
			return WriteInt(nData);
		}

		public bool WriteString(string szData)
		{
			byte[] pData = Encoding.UTF8.GetBytes(szData);
			return WriteData(pData, (UInt32)pData.Length);
		}

		ENetStreamType m_eType;
		byte[] m_pData;
		UInt32 m_dwLen;
		BinaryReader m_pReader;
		BinaryWriter m_pWrite;
	};
}
