using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace FxNet
{
	public abstract class IFxClientSocket
	{
		protected Socket m_hSocket;
		protected bool m_bReconnect;
		protected string m_szIp;
		protected int m_nPort;
		public const int BUFFER_SIZE = 64 * 1024;
		public byte[] m_pDataBuffer;
		protected DataBuffer m_pRecvBuffer;
		protected DataBuffer m_pSendBuffer;
		protected IFxDataHeader m_pDataHeader;

		public abstract bool Init(string szIp, int nPort, bool bReconnect);

		/// <summary>
		/// 这个是在线程中执行的 要注意
		/// </summary>
		public abstract void Update();

		public bool IsConnected()
		{
			return m_hSocket.Connected;
		}

		public abstract void ProcEvent(SNetEvent pEvent);

		public abstract void Connect();

		protected abstract bool CreateSocket(AddressFamily pAddressFamily);

		public abstract void OnConnect();

		public abstract void Send(byte[] byteData, UInt32 dwLen);

		protected void AsynSend(byte[] byteData, UInt32 dwLen)
		{
			// Begin sending the data to the remote device.     
			m_hSocket.BeginSend(byteData, 0, (int)dwLen, 0, new AsyncCallback(SendCallback), this);
		}
		private void SendCallback(IAsyncResult ar)
		{
			try
			{
				// Retrieve the socket from the state object.     
				IFxClientSocket pClientSocket = (IFxClientSocket)ar.AsyncState;
				// Complete sending the data to the remote device.     
				int bytesSent = pClientSocket.m_hSocket.EndSend(ar);
				OnSend((UInt32)bytesSent);
			}
			catch (SocketException e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.SocketErrorCode;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
			catch (Exception e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		internal abstract void OnSend(UInt32 bytesSent);

		public void AsynReceive()
		{
			try
			{
				// Create the state object.     
				// Begin receiving the data from the remote device.     
				m_hSocket.BeginReceive(m_pDataBuffer, 0, (int)m_pRecvBuffer.GetFreeLength(), 0, new AsyncCallback(ReceiveCallback), this);
			}
			catch(SocketException e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.SocketErrorCode;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
			catch (Exception e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}
		private void ReceiveCallback(IAsyncResult ar)
		{
			try
			{
				// Retrieve the state object and the client socket     
				// from the asynchronous state object.     
				//FxClientSocket pSocket = (FxClientSocket)ar.AsyncState;
				// Read data from the remote device.     
				int bytesRead = m_hSocket.EndReceive(ar);
				if (bytesRead < 0)
				{
					SNetEvent pEvent = new SNetEvent();
					pEvent.eType = ENetEvtType.NETEVT_ERROR;
					FxNetModule.Instance().PushNetEvent(this, pEvent);
					Disconnect();
					return;
				}
				if (bytesRead == 0)
				{
					Disconnect();
					return;
				}
				// There might be more data, so store the data received so far.     
				OnRecv(m_pDataBuffer, (UInt32)bytesRead);
				// Get the rest of the data.     
				m_hSocket.BeginReceive(m_pDataBuffer, 0, (int)m_pRecvBuffer.GetFreeLength(), 0, new AsyncCallback(ReceiveCallback), this);
			}
			catch (SocketException e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.SocketErrorCode;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
			catch (Exception e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		internal abstract void OnRecv(byte[] buffer, UInt32 bytesRead);

		public void Disconnect()
		{
			if (m_hSocket != null && m_hSocket.Connected)
			{
				m_hSocket.Disconnect(false);
				m_hSocket = null;

				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_TERMINATE;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				IoThread.Instance().DelConnectSocket(this);
			}
		}

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
				pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		string GetIPv6(string mHost, string mPort)
		{
#if UNITY_IPHONE && !UNITY_EDITOR
		string mIPv6 = getIPv6(mHost, mPort);
		return mIPv6;
#else
			return mHost + "&&ipv4";
#endif
		}
	}
}