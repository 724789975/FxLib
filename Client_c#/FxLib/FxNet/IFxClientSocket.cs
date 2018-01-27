using System;
using System.Net.Sockets;

namespace FxNet
{
	public abstract class IFxClientSocket
	{
		public const int BUFFER_SIZE = 64 * 1024;
		public byte[] m_pDataBuffer;
		protected DataBuffer m_pRecvBuffer;
		protected DataBuffer m_pSendBuffer;
		protected ISession m_pSession;

		public abstract bool Init(ISession pSession);

		/// <summary>
		/// 这个是在线程中执行的 要注意
		/// </summary>
		public abstract void Update();

		public abstract bool IsConnected();

		public abstract void ProcEvent(SNetEvent pEvent);

		public abstract void Connect(string szIp, int nPort);

		protected abstract bool CreateSocket(AddressFamily pAddressFamily);

		public abstract void OnConnect();

		public abstract void Send(byte[] byteData, UInt32 dwLen);

		protected IFxDataHeader GetDataHeader() { return m_pSession.GetDataHeader(); }

		public abstract void OnSend(UInt32 bytesSent);

		public abstract void AsynReceive();

		public abstract void OnRecv(byte[] buffer, UInt32 bytesRead);

		public abstract void Disconnect();

		protected void getIPType(String serverIp, int serverPorts, out String newServerIp, out AddressFamily mIPType)
		{
			mIPType = AddressFamily.InterNetwork;
			newServerIp = serverIp;
			try
			{
				string mIPv6 = GetIPv6(serverIp, serverPorts.ToString());
				if (!string.IsNullOrEmpty(mIPv6))
				{
					string[] m_StrTemp = System.Text.RegularExpressions.Regex.Split(mIPv6, "&&");
					if (m_StrTemp != null && m_StrTemp.Length >= 2)
					{
						string IPType = m_StrTemp[1];
						if (IPType == "ipv6")
						{
							newServerIp = m_StrTemp[0];
							mIPType = AddressFamily.InterNetworkV6;
						}
					}
				}
			}
			catch (Exception e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				//pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		string GetIPv6(string mHost, string mPort)
		{
//#if UNITY_IPHONE && !UNITY_EDITOR
//		string mIPv6 = getIPv6(mHost, mPort);
//		return mIPv6;
//#else
			return mHost + "&&ipv4";
//#endif
		}
	}
}