using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class SessionText : MonoBehaviour
{

	// Use this for initialization
	void Start ()
	{
		SessionObject pSession = H5Manager.Instance().GetServerSession();
		//pSession.m_pfOnConnect += OnConnect;
		//pSession.m_pfOnRecv += OnRecv;
		//pSession.m_pfOnError += OnError;
		//pSession.m_pfOnClose += OnClose;
		pSession.m_pfOnConnect.Add(OnConnect);
		pSession.m_pfOnRecv.Add(OnRecv);
		pSession.m_pfOnError.Add(OnError);
		pSession.m_pfOnClose.Add(OnClose);
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public void OnConnect()
	{
		m_textText.text = "onconnect";
	}
	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		string szData1 = Encoding.UTF8.GetString(pBuf);
		m_textText.text = szData1;
	}
	public void OnClose()
	{ }
	public void OnError(uint dwErrorNo)
	{ }

	public void SetPort()
	{
		ushort.TryParse(m_textPort.text, out H5Manager.Instance().m_wServerPort);
	}

	public UnityEngine.UI.Text m_textText;
	public UnityEngine.UI.Text m_textPort;
}
