using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class SessionText : MonoBehaviour
{

	// Use this for initialization
	void Start ()
	{
		m_pSession = H5Manager.Instance().GetServerSession();
		//pSession.m_pfOnConnect += OnConnect;
		//pSession.m_pfOnRecv += OnRecv;
		//pSession.m_pfOnError += OnError;
		//pSession.m_pfOnClose += OnClose;
		m_pSession.m_pfOnConnect.Add(OnConnect);
		m_pSession.m_pfOnRecv.Add(OnRecv);
		m_pSession.m_pfOnError.Add(OnError);
		m_pSession.m_pfOnClose.Add(OnClose);
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public void OnConnect()
	{
		m_textText.text = "onconnect";

		string szData = String.Format("{0}, {1}, {2}, {3}, {4}",
			"sessionobject.cs", 83, "SessionObject::OnConnect",
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		m_pSession.Send(pData, (UInt32)pData.Length);
	}
	int dw1 = 0;
	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		string szData1 = Encoding.UTF8.GetString(pBuf);
		m_textText.text = szData1;


		string szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		m_pSession.Send(pData, (UInt32)pData.Length);
	}
	public void OnClose()
	{
		m_textText.text = "on close!!!!";
		H5Helper.H5AlertString("session close");
	}
	public void OnError(uint dwErrorNo)
	{
		m_textText.text = "on error " + dwErrorNo.ToString() + "!!!!!!!";
		H5Helper.H5AlertString("session error " + dwErrorNo.ToString());
	}

	public void SetPort()
	{
		ushort.TryParse(m_textPort.text, out H5Manager.Instance().m_wServerPort);
	}

	public UnityEngine.UI.Text m_textText;
	public UnityEngine.UI.Text m_textPort;
	public SessionObject m_pSession;
}
