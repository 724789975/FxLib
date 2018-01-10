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
			throw new NotImplementedException();
		}

		internal override void OnRecv(byte[] buffer, UInt32 bytesRead)
		{
			m_pRecvBuffer.PushData(buffer, bytesRead);
			//todo 判断包长 达到一定长度 将其加入处理队列
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

			Receive();
		}

		SendState m_eSendState = SendState.SnedState_Idle;
	}
}
