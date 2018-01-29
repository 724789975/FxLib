using System;
using System.Net.Sockets;
using WebSocketSharp;

namespace FxNet
{
	public class FxWebSocket : IFxClientSocket
	{
		public override bool Init(ISession pSession)
		{
			m_pSession = pSession;
			m_pDataBuffer = new byte[BUFFER_SIZE];
			m_pRecvBuffer = new DataBuffer();
			m_pSendBuffer = new DataBuffer();
			m_pSessionBuffer = new DataBuffer();
			m_bIsConnected = false;
			m_szError = null;
			return true;
		}

		public override void Update()
		{
		}

		public override bool IsConnected()
		{
			return m_bIsConnected;
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
					break;
				default:
					break;
			}
		}

		void OnError(object pSender, ErrorEventArgs pEventArgs)
		{
			m_szError = pEventArgs.Message;
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_ERROR;
			FxNetModule.Instance().PushNetEvent(this, pEvent);
			Disconnect();
		}

		public override void Connect(string szUri, int nPort)
		{
			Disconnect();

			m_hSocket = new WebSocket(szUri);
			m_hSocket.OnMessage += (pSender, pEventArgs) => { OnRecv(pEventArgs.RawData, (UInt32)pEventArgs.RawData.Length); };
			m_hSocket.OnOpen += (pSender, pEventArgs) => { m_bIsConnected = true; OnConnect(); };
			m_hSocket.OnError += OnError;

			m_hSocket.ConnectAsync();
		}

		protected override bool CreateSocket(AddressFamily pAddressFamily)
		{
			throw new NotImplementedException();
		}

		public override void OnConnect()
		{
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_ESTABLISH;
			FxNetModule.Instance().PushNetEvent(this, pEvent);

			IoThread.Instance().AddConnectSocket(this);
		}

		public override void Send(byte[] byteData, UInt32 dwLen)
		{
			// 阻塞着发 省的包头有问题
			m_hSocket.Send(byteData);

			//byte[] pData = new byte[dwLen + GetDataHeader().GetHeaderLength()];
			//FxNet.NetStream oNetStream = new NetStream(NetStream.ENetStreamType.ENetStreamType_Write, pData, (UInt32)pData.Length);
			//UInt32 dwHeaderLen = 0;
			//byte[] pDataHeader = GetDataHeader().BuildSendPkgHeader(ref dwHeaderLen, dwLen);
			//oNetStream.WriteData(pDataHeader, GetDataHeader().GetHeaderLength());
			//oNetStream.WriteData(byteData, dwLen);
			//m_pSendBuffer.PushData(pData, (UInt32)pData.Length);
		}

		public override void OnSend(UInt32 bytesSent)
		{
			throw new NotImplementedException();
		}

		public override void AsynReceive()
		{
		}

		public override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			m_pSessionBuffer.PushData(buffer, bytesRead);
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_RECV;
			pEvent.dwValue = bytesRead;
			FxNetModule.Instance().PushNetEvent(this, pEvent);
		}

		public override void Disconnect()
		{
			if (m_hSocket != null && m_bIsConnected)
			{
				m_hSocket.Close();
				m_hSocket = null;

				m_bIsConnected = false;

				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_TERMINATE;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				IoThread.Instance().DelConnectSocket(this);
			}
		}

		bool m_bIsConnected;

		string m_szError;

		DataBuffer m_pSessionBuffer;
		protected WebSocket m_hSocket;
	}
}
