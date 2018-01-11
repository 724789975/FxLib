using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxNet
{
	public abstract class IFxDataHeader
	{
		public abstract Int32 GetHeaderLength();      // 消息头长度

		public Int32 ParsePacket(byte[] pBuf, UInt32 dwLen)
		{
			if (dwLen < GetHeaderLength())
			{
				return 0;
			}

			Int32 iPkgLen = __CheckPkgHeader(pBuf);

			return iPkgLen;
		}

		public abstract void BuildSendPkgHeader(ref UInt32 dwHeaderLen, UInt32 dwDataLen);
		public abstract Int32 __CheckPkgHeader(byte[] pBuf);
	}

	public class BinaryDataHeader : IFxDataHeader
	{
		public override void BuildSendPkgHeader(ref UInt32 dwHeaderLen, UInt32 dwDataLen)
		{
			NetStream oNetStream = new NetStream(m_dataSendBuffer, 8);
			oNetStream.WriteInt(dwDataLen);
			oNetStream.WriteInt(s_dwMagic);
			dwHeaderLen = s_dwHeaderLen;
		}

		public override Int32 GetHeaderLength()
		{
			return (Int32)s_dwHeaderLen;
		}

		public override Int32 __CheckPkgHeader(byte[] pBuf)
		{
			NetStream oNetStream = new NetStream(NetStream.ENetStreamType.ENetStreamType_Read, pBuf, s_dwHeaderLen);
			UInt32 dwLength = 0;
			oNetStream.ReadInt(ref dwLength);
			UInt32 dwMagic = 0;
			oNetStream.ReadInt(ref dwMagic);
			if (dwMagic != s_dwMagic)
			{
				return -1;
			}
			return (Int32)(s_dwHeaderLen + dwLength);
		}

		public const UInt32 s_dwHeaderLen = 8;
		// 消息头 为网络字节序
		byte[] m_dataSendBuffer = new byte[s_dwHeaderLen];
		public const UInt32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
	}
}
