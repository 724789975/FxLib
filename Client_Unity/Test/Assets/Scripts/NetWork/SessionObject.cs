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
		m_pClientSocket = m_pSession.Reconnect();
	}
	
	// Update is called once per frame
	void Update ()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		if(m_pClientSocket != null)
		{
			m_pClientSocket.Update();
		}
#else
#endif
	}

	public void OnClose()
	{
		m_textText.text = "session closed";
		//Debug.LogError("session closed");
	}

	public bool OnDestroy()
	{
		return false;
	}

	public void OnError(uint dwErrorNo)
	{
		m_textText.text = "session error " + dwErrorNo.ToString();
		//Debug.LogError("session error : " + dwErrorNo.ToString());
	}

	public bool Send(byte[] pBuf, uint dwLen)
	{
		return m_pSession.Send(pBuf, dwLen);
	}

	public void OnConnect()
	{
		m_textText.text = "server connected!!!!!";
		//var st = new System.Diagnostics.StackTrace();
		//var frame = st.GetFrame(0);
		//string szData = String.Format("{0}, {1}, {2}, {3}, {4}",
		//	frame.GetFileName(), frame.GetFileLineNumber().ToString(),
		//	frame.GetMethod().ToString(),
		//	ToString(), DateTime.Now.ToLocalTime().ToString());
		//byte[] pData = Encoding.UTF8.GetBytes(szData);

		string szData = String.Format("{0}, {1}, {2}, {3}, {4}",
			"sessionobject.cs", 83, "SessionObject::OnConnect",
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		Send(pData, (UInt32)pData.Length);
	}

	int dw1 = 0;
	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		string szData1 = Encoding.UTF8.GetString(pBuf);
		m_textText.text = szData1;
		//Debug.Log(szData1);

		//string szData = Encoding.UTF8.GetString(pBuf);
		//var st = new System.Diagnostics.StackTrace();
		//var frame = st.GetFrame(0);
		//szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
		//	frame.GetFileName(), frame.GetFileLineNumber().ToString(),
		//	frame.GetMethod().ToString(), dw1++,
		//	ToString(), DateTime.Now.ToLocalTime().ToString());
		//byte[] pData = Encoding.UTF8.GetBytes(szData);

		string szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		Send(pData, (UInt32)pData.Length);
	}

	public FxNet.ISession m_pSession;
	public SessionType m_eSessionType;
	public string m_szIP;
	public UInt16 m_wPort;
	public UnityEngine.UI.Text m_textText;
	FxNet.IFxClientSocket m_pClientSocket;
}
