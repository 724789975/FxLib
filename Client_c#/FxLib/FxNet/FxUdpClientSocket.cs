using System;
using System.Net.Sockets;

namespace FxNet
{
	public class FxUdpClientSocket : IFxClientSocket
	{



		public override bool Init(ISession pSession)
		{
			throw new NotImplementedException();
		}

		public override void Update()
		{
			throw new NotImplementedException();
		}

		public override bool IsConnected()
		{
			throw new NotImplementedException();
		}

		public override void ProcEvent(SNetEvent pEvent)
		{
			throw new NotImplementedException();
		}

		public override void Connect(string szIp, int nPort)
		{
			throw new NotImplementedException();
		}

		protected override bool CreateSocket(AddressFamily pAddressFamily)
		{
			throw new NotImplementedException();
		}

		public override void OnConnect()
		{
			throw new NotImplementedException();
		}

		public override void Send(byte[] byteData, UInt32 dwLen)
		{
			throw new NotImplementedException();
		}

		public override void OnSend(UInt32 bytesSent)
		{
			throw new NotImplementedException();
		}

		public override void AsynReceive()
		{
			throw new NotImplementedException();
		}

		public override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			throw new NotImplementedException();
		}

		public override void Disconnect()
		{
			throw new NotImplementedException();
		}
	}
}
