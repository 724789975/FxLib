﻿using System;
using System.Text;
using FxNet;

class WebSocketSession : ISession
{
	public WebSocketSession(SessionObject oGameObject)
	{
		m_oGameObject = oGameObject;
	}

	public override void Close()
	{
		if (m_pSocket == null)
		{
			return;
		}
#if UNITY_WEBGL && !UNITY_EDITOR
		m_pSocket.Disconnect();
#else
		IoThread.Instance().DelConnectSocket(m_pSocket);
#endif
	}

	public override void Init(string szIp, UInt16 wPort)
	{
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
		m_oGameObject.OnClose();
	}

	public override void OnConnect()
	{
		m_oGameObject.OnConnect();
	}

	public override bool OnDestroy()
	{
		return m_oGameObject.OnDestroy();
	}

	public override void OnError(uint dwErrorNo)
	{
		m_oGameObject.OnError(dwErrorNo);
	}

	public override void OnRecv(byte[] pBuf, uint dwLen)
	{
		NetStream pNetStream = new NetStream(pBuf, dwLen);

		byte[] pData = new byte[dwLen];
		pNetStream.ReadData(ref pData, dwLen);

		m_oGameObject.OnRecv(pData, dwLen);
	}

	public override IFxClientSocket Reconnect()
	{
		m_pSocket.Init(this);
		m_pSocket.Connect(m_oGameObject, m_szIp, m_wPort);
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

	SessionObject m_oGameObject;
	FxWebSocket m_pSocket;
}