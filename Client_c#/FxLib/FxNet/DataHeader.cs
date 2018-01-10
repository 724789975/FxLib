using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxNet
{
	public abstract class IFxDataHeader
	{
		public abstract int GetHeaderLength();      // 消息头长度

		public abstract int ParsePacket(char[] pBuf, UInt32 dwLen);

		public abstract void GetPkgHeader();           //有歧义了 现在只代表接收到的包头
		public abstract void BuildSendPkgHeader(ref UInt32 dwHeaderLen, UInt32 dwDataLen);
		public abstract bool BuildRecvPkgHeader(char[] pBuff, UInt32 dwLen, UInt32 dwOffset);
		public abstract int __CheckPkgHeader(char[] pBuf);
	}

	public class BinaryDataHeader : IFxDataHeader
	{
		public override bool BuildRecvPkgHeader(char[] pBuff, uint dwLen, uint dwOffset)
		{
			throw new NotImplementedException();
		}

		public override void BuildSendPkgHeader(ref uint dwHeaderLen, uint dwDataLen)
		{
			throw new NotImplementedException();
		}

		public override int GetHeaderLength()
		{
			throw new NotImplementedException();
		}

		public override void GetPkgHeader()
		{
			throw new NotImplementedException();
		}

		public override int ParsePacket(char[] pBuf, uint dwLen)
		{
			throw new NotImplementedException();
		}

		public override int __CheckPkgHeader(char[] pBuf)
		{
			throw new NotImplementedException();
		}

		// 消息头 为网络字节序
		char[] m_dataRecvBuffer = new char[8];
		char[] m_dataSendBuffer = new char[8];
		public const UInt32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
	}
}
