using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FxNet;

namespace test
{
	public class BinarySession : FxNet.ISession
	{
		public override void Close()
		{
			throw new NotImplementedException();
		}

		public override void Init()
		{
			throw new NotImplementedException();
		}

		public override bool IsConnected()
		{
			throw new NotImplementedException();
		}

		public override bool IsConnecting()
		{
			throw new NotImplementedException();
		}

		public override void OnClose()
		{
			throw new NotImplementedException();
		}

		public override void OnConnect()
		{
			throw new NotImplementedException();
		}

		public override bool OnDestroy()
		{
			throw new NotImplementedException();
		}

		public override void OnError(uint dwErrorNo)
		{
			throw new NotImplementedException();
		}

		public override void OnRecv(byte[] pBuf, uint dwLen)
		{
			throw new NotImplementedException();
		}

		public override IFxClientSocket Reconnect()
		{
			throw new NotImplementedException();
		}

		public override void Release()
		{
			throw new NotImplementedException();
		}

		public override bool Send(byte[] pBuf, uint dwLen)
		{
			throw new NotImplementedException();
		}
	}
}
