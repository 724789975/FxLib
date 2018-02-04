using System;
using System.Text;
using FxNet;

namespace test
{
	class WebSocketSession : FxNet.ISession
	{
		public override void Close()
		{
			if (m_pSocket == null)
			{
				return;
			}
			IoThread.Instance().DelConnectSocket(m_pSocket);
		}

		public override void Init(string szIp, UInt16 wPort)
		{
			//m_pSocket = new FxTcpClientSocket();
			m_pSocket = new FxWebSocket();
			m_szIp = szIp;
			m_wPort = wPort;
		}

		public override bool IsConnected()
		{
			if (m_pSocket == null)
			{
				return false;
			}

			return m_pSocket.IsConnected();
		}

		public override void OnClose()
		{
		}

		public override void OnConnect()
		{
			var st = new System.Diagnostics.StackTrace();
			var frame = st.GetFrame(0);
			string szData = String.Format("{0}, {1}, {2}, {3}, {4}",
				frame.GetFileName(), frame.GetFileLineNumber().ToString(),
				frame.GetMethod().ToString(),
				ToString(), DateTime.Now.ToLocalTime().ToString());
			byte[] pData = Encoding.UTF8.GetBytes(szData);
			Send(pData, (UInt32)pData.Length);
		}

		public override bool OnDestroy()
		{
			return false;
		}

		public override void OnError(uint dwErrorNo)
		{
		}

		UInt32 dw1 = 0;
		public override void OnRecv(byte[] pBuf, uint dwLen)
		{
			NetStream pNetStream = new NetStream(pBuf, dwLen);

			byte[] pData = new byte[dwLen];
			pNetStream.ReadData(ref pData, dwLen);

			string szData = Encoding.UTF8.GetString(pData);
			var st = new System.Diagnostics.StackTrace();
			var frame = st.GetFrame(0);
			szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
				frame.GetFileName(), frame.GetFileLineNumber().ToString(),
				frame.GetMethod().ToString(), dw1++,
				ToString(), DateTime.Now.ToLocalTime().ToString());
			pData = Encoding.UTF8.GetBytes(szData);
			Send(pData, (UInt32)pData.Length);
		}

		public override IFxClientSocket Reconnect()
		{
			m_pSocket.Init(this);
			m_pSocket.Connect(m_szIp, m_wPort);
			return m_pSocket;
		}

		public override void Release()
		{
		}

		public override bool Send(byte[] pBuf, uint dwLen)
		{
			if (IsConnected())
			{
				m_pSocket.Send(pBuf, dwLen);
				return true;
			}
			return false;
		}
		protected IFxClientSocket m_pSocket;
	}
}
