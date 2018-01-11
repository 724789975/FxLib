using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
		public override void Connect()
		{
			Disconnect();
			String newServerIp = "";
			AddressFamily pAddressFamily = AddressFamily.InterNetwork;
			getIPType(m_szIp, m_nPort, out newServerIp, out pAddressFamily);
			IPEndPoint ipe = new IPEndPoint(IPAddress.Parse(newServerIp), m_nPort);

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
				pEvent.dwValue = (UInt32)e.HResult;
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

		internal override void OnSend(UInt32 bytesSent)
		{
			m_pSendBuffer.PopData(bytesSent);
			if (m_pSendBuffer.GetUsedLength() == 0)
			{
				m_eSendState = SendState.SnedState_Idle;
			}
			else
			{
				m_eSendState = SendState.SnedState_Sending;
				Send(m_pSendBuffer.GetData(), m_pSendBuffer.GetUsedLength());
			}
		}

		public override void Update()
		{
			if (m_eSendState == SendState.SnedState_Idle)
			{
				if (m_pSendBuffer.GetUsedLength() > 0)
				{
					m_eSendState = SendState.SnedState_Sending;
					Send(m_pSendBuffer.GetData(), m_pSendBuffer.GetUsedLength());
				}
			}
		}

		public override void ProcEvent(SNetEvent pEvent)
		{
			switch (pEvent.eType)
			{
				case ENetEvtType.NETEVT_ESTABLISH:
					{
						IoThread.Instance().AddConnectSocket(this);
					}
					break;
				case ENetEvtType.NETEVT_RECV:
					{
						byte[] pData = new byte[pEvent.dwValue];
						m_pRecvBuffer.PopData(pData, pEvent.dwValue);
					}
					break;
				case ENetEvtType.NETEVT_ERROR:
					break;
				case ENetEvtType.NETEVT_TERMINATE:
					break;
				case ENetEvtType.NETEVT_RELEASE:
					break;
				default:
					break;
			}
		}

		internal override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			m_pRecvBuffer.PushData(buffer, bytesRead);
			//判断包长 达到一定长度 将其加入处理队列
			Int32 iLength = m_pDataHeader.ParsePacket(buffer, bytesRead);
			if (iLength < 0)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_ERROR;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				Disconnect();
			}
			else if (iLength > 0)
			{
				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_RECV;
				pEvent.dwValue = (UInt32)iLength;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
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

			//Receive();		//这是在主线程 所以不能receive
		}

		public override bool Init(string szIp, int nPort, bool bReconnect)
		{
			m_szIp = szIp;
			m_nPort = nPort;
			m_bReconnect = bReconnect;
			m_pDataBuffer = new byte[BUFFER_SIZE];
			m_pRecvBuffer = new DataBuffer();
			m_pSendBuffer = new DataBuffer();
			m_pDataHeader = new BinaryDataHeader();
			return true;
		}

		SendState m_eSendState = SendState.SnedState_Idle;
	}
}
