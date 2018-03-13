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
		//m_pSession.m_pfOnRecv.Add(OnRecv);
		m_pSession.m_pfOnError.Add(OnError);
		m_pSession.m_pfOnClose.Add(OnClose);

		//m_pSession.RegistMessage(GameProto.PlayerRequestGameTest.Descriptor.FullName, OnTest);
		m_pSession.RegistMessage(typeof(GameProto.PlayerRequestGameTest).FullName, OnTest);
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public void OnConnect()
	{
		m_textText.text = "onconnect";

		GameProto.PlayerRequestGameTest oTest = new GameProto.PlayerRequestGameTest();

		oTest.SzTest = String.Format("{0}, {1}, {2}, {3}, {4}",
			"sessionobject.cs", 83, "SessionObject::OnConnect",
			ToString(), DateTime.Now.ToLocalTime().ToString());

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString(GameProto.PlayerRequestGameTest.Descriptor.FullName);
		byte[] pProto = new byte[oTest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
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

	int dw1 = 0;
	public void OnTest(byte[] pBuf)
	{
		GameProto.PlayerRequestGameTest oTest = GameProto.PlayerRequestGameTest.Parser.ParseFrom(pBuf);
		if (oTest == null)
		{
			H5Helper.H5LogStr("OnTest error parse");
			return;
		}

		m_textText.text = oTest.SzTest;

		oTest.SzTest = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString(GameProto.PlayerRequestGameTest.Descriptor.FullName);
		byte[] pProto = new byte[oTest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public UnityEngine.UI.Text m_textText;
	public UnityEngine.UI.Text m_textPort;
	public SessionObject m_pSession;
}
