using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace FxNet
{
	public class FxUdpClientSocket : IFxClientSocket
	{
		public override void Connect()
		{
			throw new NotImplementedException();
		}

		public override void ProcEvent(SNetEvent pEvent)
		{
			throw new NotImplementedException();
		}

		public override void Update()
		{
			throw new NotImplementedException();
		}

		internal override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			throw new NotImplementedException();
		}
		internal override void OnSend(UInt32 bytesSent)
		{
			throw new NotImplementedException();
		}

		protected override bool CreateSocket(AddressFamily pAddressFamily)
		{
			throw new NotImplementedException();
		}
	}
}
