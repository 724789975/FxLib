using System;
using System.Net;
using System.Net.Sockets;
using WebSocketSharp;

namespace FxNet
{
	public class FxWebSocket : FxNet.IFxClientSocket
	{
		enum SendState
		{
			SendState_None,
			//SnedState_NeedSend,			//主线程修改为这个状态 表示需要发送
			SnedState_Idle,             //发送静止
			SnedState_Sending,          //正在发送中
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

		private void ConnectCallback(IAsyncResult ar)
		{ }
		public override void Connect(string szUri, int nPort)
		{
			Disconnect();

			m_hSocket = new WebSocket(szUri);
			m_hSocket.OnMessage += (pSender, pEventArgs) => { OnRecv(pEventArgs.RawData, (UInt32)pEventArgs.RawData.Length); };
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
			m_pSessionBuffer.PushData(buffer, bytesRead);
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_RECV;
			pEvent.dwValue = bytesRead;
			FxNetModule.Instance().PushNetEvent(this, pEvent);
		}

		public override void Disconnect()
		{
			throw new NotImplementedException();
		}

		SendState m_eSendState = SendState.SnedState_Idle;

		DataBuffer m_pSessionBuffer;
		protected WebSocket m_hSocket;
	}
}
