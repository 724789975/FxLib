using System;
using System.Text;
using FxNet;

public class BinarySession : ISession
{
	public BinarySession(SessionObject oGameObject)
	{
		m_oGameObject = oGameObject;
	}

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
		m_pDataHeader = new BinaryDataHeader();
		m_pSocket = new FxTcpClientSocket();
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
		UInt32 dwDataLen = 0;
		pNetStream.ReadInt(ref dwDataLen);
		UInt32 dwMagicNum = 0;
		pNetStream.ReadInt(ref dwMagicNum);

		byte[] pData = new byte[dwDataLen];
		pNetStream.ReadData(ref pData, dwDataLen);

		m_oGameObject.OnRecv(pData, dwDataLen);
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

	SessionObject m_oGameObject;
	protected IFxClientSocket m_pSocket;
}
