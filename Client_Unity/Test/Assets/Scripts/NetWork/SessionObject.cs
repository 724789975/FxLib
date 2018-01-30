using System;
using System.Text;
using UnityEngine;

public class SessionObject : MonoBehaviour
{
	public enum SessionType
	{
		SessionType_TCP,
		SessionType_WebSocket,
	}
	// Use this for initialization
	void Start ()
	{
		switch (m_eSessionType)
		{
			case SessionType.SessionType_TCP:
				{
					m_pSession = new BinarySession(this);
				}
				break;
			case SessionType.SessionType_WebSocket:
				{
					m_pSession = new WebSocketSession(this);
				}
				break;
			default:
				{
					throw new ArgumentException("error session type : " + m_eSessionType.ToString());
				}
		}

		m_pSession.Init(m_szIP, m_wPort);
		m_pSession.Reconnect();
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public void OnClose()
	{
	}

	public bool OnDestroy()
	{
		return false;
	}

	public void OnError(uint dwErrorNo)
	{
		Debug.LogError("session error : " + dwErrorNo.ToString());
	}

	public bool Send(byte[] pBuf, uint dwLen)
	{
		return m_pSession.Send(pBuf, dwLen);
	}

	public void OnConnect()
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

	int dw1 = 0;
	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		string szData1 = Encoding.UTF8.GetString(pBuf);
		Debug.Log(szData1);

		string szData = Encoding.UTF8.GetString(pBuf);
		var st = new System.Diagnostics.StackTrace();
		var frame = st.GetFrame(0);
		szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			frame.GetFileName(), frame.GetFileLineNumber().ToString(),
			frame.GetMethod().ToString(), dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		Send(pData, (UInt32)pData.Length);
	}

	public FxNet.ISession m_pSession;
	public SessionType m_eSessionType;
	public string m_szIP;
	public UInt16 m_wPort;
}
