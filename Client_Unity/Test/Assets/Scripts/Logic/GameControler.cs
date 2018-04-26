using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class GameControler : MonoBehaviour
{

	// Use this for initialization
	void Start()
	{
		H5Manager.Instance().GetGameSessionResetCallBack().Add(OnLoginSessionReset);
	}

	public void OnLoginSessionReset(SessionObject obj)
	{
		m_pSession = H5Manager.Instance().GetLoginSession();
		m_pSession.m_pfOnConnect.Add(OnConnect);
		m_pSession.m_pfOnError.Add(OnError);
		m_pSession.m_pfOnClose.Add(OnClose);

		//m_pSession.RegistMessage("GameProto.PlayerRequestGameTest", OnTest);
		//m_pSession.RegistMessage("GameProto.LoginAckPlayerServerId", OnLoginAckPlayerServerId);
		//m_pSession.RegistMessage("GameProto.LoginAckPlayerLoginResult", OnLoginAckPlayerLoginResult);
		//m_pSession.RegistMessage("GameProto.LoginAckPlayerMakeTeam", OnLoginAckPlayerMakeTeam);
		//m_pSession.RegistMessage("GameProto.LoginAckPlayerGameStart", OnLoginAckPlayerGameStart);
	}

	// Update is called once per frame
	void Update()
	{

	}

	public void OnConnect()
	{
		H5Helper.H5LogStr("game connected");
		//GameProto.PlayerRequestLoginServerId oTeam = new GameProto.PlayerRequestLoginServerId();
		//byte[] pData = new byte[1024];
		//FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		//pStream.WriteString("GameProto.PlayerRequestLoginServerId");
		//byte[] pProto = new byte[oTeam.CalculateSize()];
		//Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		//oTeam.WriteTo(oStream);
		//pStream.WriteData(pProto, (uint)pProto.Length);

		//m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
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

	public void OnDestroy()
	{
		if (m_pSession != null)
		{
			m_pSession.m_pfOnConnect.Remove(OnConnect);
			m_pSession.m_pfOnError.Remove(OnError);
			m_pSession.m_pfOnClose.Remove(OnClose);

			m_pSession.UnRegistMessage("GameProto.PlayerRequestGameTest");
		}
		H5Manager.Instance().m_setLoginSessionResetCallBack.Remove(OnLoginSessionReset);
	}

	public UnityEngine.UI.Text m_textText;
	public UnityEngine.UI.Text m_textPort;
	public SessionObject m_pSession;
}
