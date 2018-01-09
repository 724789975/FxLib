using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxNet
{
	public class FxUdpClientSocket : FxClientSocket
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

		internal override void OnRecv(byte[] buffer, int bytesRead)
		{
			throw new NotImplementedException();
		}

		internal override void OnSend(int bytesSent)
		{
			throw new NotImplementedException();
		}
	}
}
