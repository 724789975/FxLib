using System;
using System.Net;
using System.Net.Sockets;

namespace FxNet
{
	public class FxTcpClientSocket : IFxClientSocket
    {
		enum SendState
		{
			SendState_None,
			//SnedState_NeedSend,			//主线程修改为这个状态 表示需要发送
			SnedState_Idle,				//发送静止
			SnedState_Sending,			//正在发送中
		}
		public override void Connect(string szIp, int nPort)
		{
			Disconnect();
			String newServerIp = "";
			AddressFamily pAddressFamily = AddressFamily.InterNetwork;
			getIPType(szIp, nPort, out newServerIp, out pAddressFamily);
			IPEndPoint ipe = new IPEndPoint(IPAddress.Parse(newServerIp), nPort);

			CreateSocket(pAddressFamily);
			try
			{
				m_hSocket.BeginConnect(ipe, new AsyncCallback(ConnectCallback), this);
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
				//pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
			}
		}
		private void ConnectCallback(IAsyncResult ar)
		{
			try
			{
				FxTcpClientSocket pClientSocket = (FxTcpClientSocket)ar.AsyncState;
				m_hSocket.EndConnect(ar);

				OnConnect();
			}
			catch (SocketException e)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				pEvent.dwValue = (UInt32)e.SocketErrorCode;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
			}
		}

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
				FxTcpClientSocket pClientSocket = (FxTcpClientSocket)ar.AsyncState;
				// Complete sending the data to the remote device.     
				int bytesSent = m_hSocket.EndSend(ar);
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
				//pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		public override void OnSend(UInt32 bytesSent)
		{
			m_pSendBuffer.PopData(bytesSent);
			if (m_pSendBuffer.GetUsedLength() == 0)
			{
				m_eSendState = SendState.SnedState_Idle;
			}
			else
			{
				m_eSendState = SendState.SnedState_Sending;
				AsynSend(m_pSendBuffer.GetData(), m_pSendBuffer.GetUsedLength());
			}
		}

		public override void Update()
		{
			if (m_eSendState == SendState.SnedState_Idle)
			{
				if (m_pSendBuffer.GetUsedLength() > 0)
				{
					m_eSendState = SendState.SnedState_Sending;
					AsynSend(m_pSendBuffer.GetData(), m_pSendBuffer.GetUsedLength());
				}
			}
		}

		public override void ProcEvent(SNetEvent pEvent)
		{
			switch (pEvent.eType)
			{
				case ENetEvtType.NETEVT_ESTABLISH:
					{
						m_pSession.OnConnect();
						IoThread.Instance().AddConnectSocket(this);
					}
					break;
				case ENetEvtType.NETEVT_RECV:
					{
						m_pSession.OnRecv(m_pSessionBuffer.GetData(), pEvent.dwValue);
						m_pSessionBuffer.PopData(pEvent.dwValue);
					}
					break;
				case ENetEvtType.NETEVT_ERROR:
					{
						m_pSession.OnError(pEvent.dwValue);
					}
					break;
				case ENetEvtType.NETEVT_TERMINATE:
					{
						m_pSession.OnClose();
						SNetEvent oEvent = new SNetEvent();
						oEvent.eType = ENetEvtType.NETEVT_RELEASE;
						FxNetModule.Instance().PushNetEvent(this, oEvent);
					}
					break;
				case ENetEvtType.NETEVT_RELEASE:
                    m_pSession.OnDestroy();
					break;
				default:
					break;
			}
		}

		public override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			m_pRecvBuffer.PushData(buffer, bytesRead);
			//判断包长 达到一定长度 将其加入处理队列
			while (true)
			{
				Int32 iLength = GetDataHeader().ParsePacket(m_pRecvBuffer.GetData(), m_pRecvBuffer.GetUsedLength());
				if (iLength == 0)
				{
					break;
				}
				if (iLength < 0)
				{
					SNetEvent pEvent = new SNetEvent();
					pEvent.eType = ENetEvtType.NETEVT_ERROR;
					FxNetModule.Instance().PushNetEvent(this, pEvent);
					Disconnect();
					return;
				}
				else if (iLength > 0)
				{
					if (m_pRecvBuffer.GetUsedLength() < iLength)
					{
						break;
					}
					m_pSessionBuffer.PushData(m_pRecvBuffer.GetData(), (UInt32)iLength);
					m_pRecvBuffer.PopData((UInt32)iLength);
					SNetEvent pEvent = new SNetEvent();
					pEvent.eType = ENetEvtType.NETEVT_RECV;
					pEvent.dwValue = (UInt32)iLength;
					FxNetModule.Instance().PushNetEvent(this, pEvent);
				}
			}
		}

		protected override bool CreateSocket(AddressFamily pAddressFamily)
		{
			m_hSocket = new Socket(pAddressFamily, SocketType.Stream, ProtocolType.Tcp);
			return true;
		}

		public override void OnConnect()
		{
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_ESTABLISH;
			FxNetModule.Instance().PushNetEvent(this, pEvent);
		}

		public override bool Init(ISession pSession)
		{
			m_pSession = pSession;
			m_pDataBuffer = new byte[BUFFER_SIZE];
			m_pRecvBuffer = new DataBuffer();
			m_pSendBuffer = new DataBuffer();
			m_pSessionBuffer = new DataBuffer();
			return true;
		}

		public override void Send(byte[] byteData, UInt32 dwLen)
		{
			byte[] pData = new byte[dwLen + GetDataHeader().GetHeaderLength()];
			FxNet.NetStream oNetStream = new NetStream(NetStream.ENetStreamType.ENetStreamType_Write, pData, (UInt32)pData.Length);
			UInt32 dwHeaderLen = 0;
			byte[] pDataHeader = GetDataHeader().BuildSendPkgHeader(ref dwHeaderLen, dwLen);
			oNetStream.WriteData(pDataHeader, GetDataHeader().GetHeaderLength());
			oNetStream.WriteData(byteData, dwLen);
			m_pSendBuffer.PushData(pData, (UInt32)pData.Length);
		}

		public override bool IsConnected()
		{
			return m_hSocket.Connected;
		}

		public override void AsynReceive()
		{
			try
			{
				// Create the state object.     
				// Begin receiving the data from the remote device.     
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
				//pEvent.dwValue = (UInt32)e.HResult;
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
				//pEvent.dwValue = (UInt32)e.HResult;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
				return;
			}
		}

		public override void Disconnect()
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

		SendState m_eSendState = SendState.SnedState_Idle;

		DataBuffer m_pSessionBuffer;
		protected Socket m_hSocket;
	}
}
