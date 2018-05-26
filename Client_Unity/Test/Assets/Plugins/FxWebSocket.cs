using System;
using System.Text;
using System.Net.Sockets;
using System.Collections;
using System.Runtime.InteropServices;
using UnityEngine;

#if UNITY_WEBGL && !UNITY_EDITOR
public class WebSocket
{
	private Uri mUrl;
    byte[] buffer = null;
	public WebSocket(Uri url)
	{
		mUrl = url;
        buffer = new byte[128 * 1024];
		string protocol = mUrl.Scheme;
		if (!protocol.Equals("ws") && !protocol.Equals("wss"))
			throw new ArgumentException("Unsupported protocol: " + protocol);
	}

	public void SendString(string str)
	{
		Send(Encoding.UTF8.GetBytes (str), 0);
	}

	public string RecvString()
	{
        int len = 0;
		byte[] retval = Recv(0, ref len);
		if (retval == null)
			return null;
		return Encoding.UTF8.GetString (retval);
	}

	[DllImport("__Internal")]
	private static extern int SocketCreate (string url);

	[DllImport("__Internal")]
	private static extern int SocketState (int socketInstance);

	[DllImport("__Internal")]
	private static extern void SocketSend (int socketInstance, byte[] ptr, int length);

	[DllImport("__Internal")]
	private static extern void SocketRecv (int socketInstance, byte[] ptr, int length);

	[DllImport("__Internal")]
	private static extern int SocketRecvLength (int socketInstance);

	[DllImport("__Internal")]
	private static extern void SocketClose (int socketInstance);

	[DllImport("__Internal")]
	private static extern int SocketError (int socketInstance, byte[] ptr, int length);

	int m_NativeRef = 0;

	public void Send(byte[] buffer, int size)
	{
		SocketSend (m_NativeRef, buffer, size);
	}

	public byte[] Recv(int recv_size, ref int len)
	{
		int length = SocketRecvLength (m_NativeRef);
		if (length == 0)
			return null;
        if (recv_size < length)
        {
            length = recv_size;
        }
		//byte[] buffer = new byte[length];
		SocketRecv (m_NativeRef, buffer, length);
        len = length;
		return buffer;
	}

	public IEnumerator Connect()
	{
		m_NativeRef = SocketCreate(mUrl.ToString());

		while (SocketState(m_NativeRef) == 0)
			yield return 0;

		m_pfOnConnect();
	}
 
	public void Close()
	{
		SocketClose(m_NativeRef);
	}

	public string error
	{
		get {
			const int bufsize = 1024;
			byte[] buffer = new byte[bufsize];
			int result = SocketError (m_NativeRef, buffer, bufsize);

			if (result == 0)
				return null;

			return Encoding.UTF8.GetString (buffer);				
		}
	}
    public bool IsConnected
    {
        get { return (SocketState(m_NativeRef) != 0); }
    }

	public Action m_pfOnConnect;
}
namespace FxNet
{
	public class FxWebSocket : IFxClientSocket
	{
		public override bool Init(ISession pSession)
		{
			m_pSession = pSession;
			//m_pDataBuffer = new byte[BUFFER_SIZE];
			//m_pRecvBuffer = new DataBuffer();
			//m_pSendBuffer = new DataBuffer();
			//m_pSessionBuffer = new DataBuffer();
			return true;
		}

		public override void Update()
		{
			if (!IsConnected())
			{
				return;
			}
			int nLen = 0;
			byte[] retVal = null;
			try
			{
				retVal = m_hSocket.Recv(64 * 1024, ref nLen);
			}
			catch(SocketException e)
			{
				switch ((SocketError)e.ErrorCode)
				{
					case SocketError.TryAgain:
					case SocketError.TimedOut:
					case SocketError.NoBufferSpaceAvailable:
					case SocketError.Interrupted:
					case SocketError.IOPending:
					case SocketError.WouldBlock:
					return;
				}
				m_pSession.OnError((UInt32)e.ErrorCode);
				Disconnect();
			}
			catch(Exception)
			{
				Disconnect();
			}
			if (retVal == null)
			{
				return;
			}

			OnRecv(retVal, (UInt32)nLen);
		}

		public override bool IsConnected()
		{
			if (m_hSocket == null)
			{
				return false;
			}
			bool bConnected = m_hSocket.IsConnected && (GetError() == null);
			if (m_bConnected && !bConnected)
			{
				m_bConnected = false;
				if (m_hSocket != null)
				{
					Disconnect();
				}
				else
				{
					m_pSession.OnClose();
				}
			}
			return bConnected;
		}

		public override void ProcEvent(SNetEvent pEvent)
		{
			throw new NotImplementedException();
		}

		public WebSocket GetWebSocket() { return m_hSocket; }

		public void Connect(MonoBehaviour pSession, string szIp, int nPort)
		{
			Disconnect();

			Uri pUrl = new Uri("ws://" + szIp + ":" + nPort + "/");
			string szProtocol = pUrl.Scheme;
			if (!szProtocol.Equals("ws") && !szProtocol.Equals("wss"))
			{
				throw new ArgumentException("Unsupported protocol: " + szProtocol);
			}

			m_hSocket = new WebSocket(pUrl);
			m_hSocket.m_pfOnConnect = OnConnect;

			pSession.StartCoroutine(m_hSocket.Connect());
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
			m_bConnected = true;
			m_pSession.OnConnect();
		}

		public override void Send(byte[] byteData, UInt32 dwLen)
		{
			try
			{
				// 阻塞着发 省的包头有问题
				m_hSocket.Send(byteData, (Int32)dwLen);
			}
			catch (SocketException e)
			{
				m_pSession.OnError((UInt32)e.ErrorCode);
				Disconnect();
			}
		}

		public override void OnSend(UInt32 bytesSent)
		{
			throw new NotImplementedException();
		}

		public override void AsynReceive()
		{
			throw new NotImplementedException();
		}

		public override void OnRecv(byte[] bytesBuffer, UInt32 bytesRead)
		{
			m_pSession.OnRecv(bytesBuffer, bytesRead);
		}

		public override void Disconnect()
		{
			try
			{
				if (m_hSocket != null)
				{
					m_bConnected = false;
                    m_hSocket.Close();
					m_hSocket = null;

					m_pSession.OnClose();
				}
			}
			catch (Exception)
			{
			}
		}

		public string GetError()
		{
			return m_hSocket.error;
		}


		DataBuffer m_pSessionBuffer = null;
		protected WebSocket m_hSocket = null;
		bool m_bConnected = false;
	}
}
#else
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
                    m_pSession.OnDestroy();
					break;
				default:
					break;
			}
		}

		void OnError(object pSender, WebSocketSharp.ErrorEventArgs pEventArgs)
		{
			m_szError = pEventArgs.Message;
			SNetEvent pEvent = new SNetEvent();
			pEvent.eType = ENetEvtType.NETEVT_ERROR;
			FxNetModule.Instance().PushNetEvent(this, pEvent);
			Disconnect();
		}

		public void Connect(MonoBehaviour pSession, string szIp, int nPort)
		{
			Connect(szIp, nPort);
		}

		public override void Connect(string szIp, int nPort)
		{
			Disconnect();

			Uri pUrl = new Uri("ws://" + szIp + ":" + nPort + "/");
			string szProtocol = pUrl.Scheme;
			if (!szProtocol.Equals("ws") && !szProtocol.Equals("wss"))
			{
				throw new ArgumentException("Unsupported protocol: " + szProtocol);
			}

			m_hSocket = new WebSocketSharp.WebSocket(pUrl.ToString());
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
            byte[] byteSend = new byte[dwLen];
            Array.Copy(byteData, byteSend, dwLen);
			m_hSocket.Send(byteSend);
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
                WebSocketSharp.WebSocket pSock = m_hSocket;

				m_bIsConnected = false;
				m_hSocket = null;
                pSock.Close();

				SNetEvent pEvent = new SNetEvent();
				pEvent.eType = ENetEvtType.NETEVT_TERMINATE;
				FxNetModule.Instance().PushNetEvent(this, pEvent);
				IoThread.Instance().DelConnectSocket(this);
			}
		}

		public string GetError()
		{
			return m_szError;
		}

		bool m_bIsConnected;

		string m_szError;

		DataBuffer m_pSessionBuffer;
		protected WebSocketSharp.WebSocket m_hSocket;
	}
}
#endif
