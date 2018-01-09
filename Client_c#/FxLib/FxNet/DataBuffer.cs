using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
			lock(m_obj)
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
			lock(m_obj)
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
}
