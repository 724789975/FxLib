using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

//{"code":200,"token":"asdfghjk","data":{"id":372593,"nick_name":"test","avatar":"no pic","sex":1,"balance":0},"descrp":"\u767b\u5f55\u6210\u529f"}
[Serializable]
class RoleData
{
	public UInt64 id = 0;
	public string nick_name = "";
	public string avatar = "";
	public uint sex = 0;
	public uint balance = 0;
}
[Serializable]
class RoleDataRet
{
	public int code = 0;
	public string token = "";
	public uint last_login_server_id = 0;
	public RoleData data = new RoleData();
	public UInt64 team_id = 0;
	public string game_ip = "";
	public uint game_port = 0;
	public string descrp = "";
}

public class LoginControler : SingletonObject<LoginControler>
{
	// Use this for initialization
	void Start()
	{
		CreateInstance(this);
		DontDestroyOnLoad(this);
		H5Manager.Instance().GetLoginSessionResetCallBack().Add(OnLoginSessionReset);
	}

	public void OnLoginSessionReset(SessionObject obj)
	{
		m_pSession = obj;
		m_pSession.m_pfOnConnect.Add(OnConnect);
		m_pSession.m_pfOnError.Add(OnError);
		m_pSession.m_pfOnClose.Add(OnClose);

		m_pSession.RegistMessage("GameProto.LoginAckPlayerServerId", OnLoginAckPlayerServerId);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerLoginResult", OnLoginAckPlayerLoginResult);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerMakeTeam", OnLoginAckPlayerMakeTeam);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerInviteTeam", OnLoginAckPlayerInviteTeam);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerGameStart", OnLoginAckPlayerGameStart);
		m_pSession.RegistMessage("GameProto.LoginNotifyPlayerGameKick", OnLoginNotifyPlayerGameKick);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerOnLinePlayer", OnLoginAckPlayerOnLinePlayer);
		m_pSession.RegistMessage("GameProto.LoginNotifyPlayerInviteTeam", OnLoginNotifyPlayerInviteTeam);
		m_pSession.RegistMessage("GameProto.LoginNotifyPlayerTeamInfo", OnLoginNotifyPlayerTeamInfo);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerEnterTeam", OnLoginAckPlayerEnterTeam);
		m_pSession.RegistMessage("GameProto.LoginNotifyPlayerRefuseEnterTeam", OnLoginNotifyPlayerRefuseEnterTeam);
		m_pSession.RegistMessage("GameProto.LoginAckPlayerLeaveTeam", OnLoginAckPlayerLeaveTeam);
    }

	// Update is called once per frame
	void Update()
	{

	}

	public void OnConnect()
	{
		GameProto.PlayerRequestLoginServerId oTeam = new GameProto.PlayerRequestLoginServerId();
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginServerId");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void OnClose()
	{
		SampleDebuger.LogError("session close");
	}
	public void OnError(uint dwErrorNo)
	{
		SampleDebuger.LogError("session error " + dwErrorNo.ToString());
	}

	public void SetPort()
	{
		//ushort.TryParse(m_textPort.text, out H5Manager.Instance().m_wLoginPort);
	}

	public void MakeTeam()
	{
		GameProto.PlayerRequestLoginMakeTeam oTeam = new GameProto.PlayerRequestLoginMakeTeam();
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginMakeTeam");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void LeaveTeam()
	{
		GameProto.PlayerRequestLoginLeaveTeam oTeam = new GameProto.PlayerRequestLoginLeaveTeam();
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginLeaveTeam");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}
	
	public void TeamStart()
	{
		GameProto.PlayerRequestLoginGameStart oTeam = new GameProto.PlayerRequestLoginGameStart();
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginGameStart");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void OnlinePlayers()
	{
		GameProto.PlayerRequestLoginOnLinePlayer oTeam = new GameProto.PlayerRequestLoginOnLinePlayer();
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginOnLinePlayer");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void OnLoginAckPlayerServerId(byte[] pBuf)
	{
		GameProto.LoginAckPlayerServerId oRet = GameProto.LoginAckPlayerServerId.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerServerId error parse");
			return;
		}

		string szUrl = GameInstance.Instance().proUrlHost + GameInstance.Instance().proGetRoleUri;
		WWWForm form = new WWWForm();
		form.AddField("platform", GameInstance.Instance().proPlatform);
		form.AddField("name", GameInstance.Instance().proName);
		form.AddField("head_img", GameInstance.Instance().proHeadImage);
		form.AddField("sex", GameInstance.Instance().proSex.ToString());
		form.AddField("access_token", GameInstance.Instance().proAccessToken);
		form.AddField("expires_date", GameInstance.Instance().proExpiresDate.ToString());
		form.AddField("openid", GameInstance.Instance().proOpenId);
		form.AddField("server_id", oRet.DwServerId.ToString());

		StartCoroutine(H5Helper.SendPost(szUrl, form, OnRoleData));
	}
	public void OnLoginAckPlayerLoginResult(byte[] pBuf)
	{
		GameProto.LoginAckPlayerLoginResult oRet = GameProto.LoginAckPlayerLoginResult.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnTest error parse");
			return;
		}

		SampleDebuger.Log("login ret : " + oRet.DwResult.ToString());

		AssetBundleLoader.Instance().LoadLevelAsset("lobby", delegate()
			{
				if (!string.IsNullOrEmpty(GameInstance.Instance().proGameIp))
				{
					H5Manager.Instance().ConnectGame(GameInstance.Instance().proGameIp, GameInstance.Instance().proGamePort);
				}
			}
		);
		}

	public void OnLoginAckPlayerMakeTeam(byte[] pBuf)
	{
		GameProto.LoginAckPlayerMakeTeam oRet = GameProto.LoginAckPlayerMakeTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerMakeTeam error parse");
			return;
		}

		SampleDebuger.Log("make team ret : " + oRet.DwResult.ToString() + " slot : " + oRet.DwSlotId.ToString() + " team_id : " + oRet.QwTeamId.ToString());
	}
	public void OnLoginAckPlayerGameStart(byte[] pBuf)
	{
		GameProto.LoginAckPlayerGameStart oRet = GameProto.LoginAckPlayerGameStart.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerGameStart error parse");
			return;
		}
		SampleDebuger.Log("game start : " + oRet.DwResult.ToString());

		H5Manager.Instance().ConnectGame(oRet.SzListenIp, (ushort)oRet.DwPlayerPort);
	}

	public void OnLoginAckPlayerInviteTeam(byte[] pBuf)
	{
		GameProto.LoginAckPlayerInviteTeam oRet = GameProto.LoginAckPlayerInviteTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerInviteTeam error parse");
			return;
		}
		SampleDebuger.Log("invite team ret : " + oRet.DwResult.ToString());
	}

	public void OnLoginNotifyPlayerGameKick(byte[] pBuf)
	{
		GameProto.LoginNotifyPlayerGameKick oRet = GameProto.LoginNotifyPlayerGameKick.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginNotifyPlayerGameKick error parse");
			return;
		}
		SampleDebuger.Log("game kick ret : " + oRet.DwResult.ToString());
	}

	public void OnLoginAckPlayerOnLinePlayer(byte[] pBuf)
	{
		GameProto.LoginAckPlayerOnLinePlayer oRet = GameProto.LoginAckPlayerOnLinePlayer.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerOnLinePlayer error parse");
			return;
		}
		SampleDebuger.Log("online player ret : " + oRet.DwResult.ToString());

		AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szPlayerList), GameObjectConstant.g_szPlayerList, delegate (UnityEngine.Object ob)
			{
				RoleList pRoleList = RoleList.CreateInstance(ob, MainCanvas.Instance().transform);
				pRoleList.SetPlayerIds(oRet.QwPlayerId);
			}
		);
	}

	public void OnLoginNotifyPlayerInviteTeam(byte[] pBuf)
	{
		GameProto.LoginNotifyPlayerInviteTeam oRet = GameProto.LoginNotifyPlayerInviteTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerOnLinePlayer error parse");
			return;
		}

		SampleDebuger.Log("on invitee team player id : " + oRet.QwPlayerId.ToString() + " team id : " + oRet.QwTeamId.ToString());

		AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szControlPanel), GameObjectConstant.g_szControlPanel, delegate (UnityEngine.Object ob)
			{
				GameObject go_RoleList = Instantiate((GameObject)ob, MainCanvas.Instance().transform);
				go_RoleList.GetComponent<ControlPanel>().Init("player : " + oRet.QwPlayerId.ToString() + " invite you to team :" + oRet.QwTeamId.ToString(),
					IntoInviteTeam, oRet, NotIntoInviteTeam, oRet);
			}
		);
	}

	public void OnLoginNotifyPlayerTeamInfo(byte[] pBuf)
	{
		GameProto.LoginNotifyPlayerTeamInfo oRet = GameProto.LoginNotifyPlayerTeamInfo.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginNotifyPlayerTeamInfo error parse");
			return;
		}
		string szContent = "OnLoginNotifyPlayerTeamInfo team id : " + oRet.QwTeamId.ToString() + "\n";
		for (int i = 0; i < oRet.TeamRoleData.Count; ++i)
		{
			szContent += "playerid : " + oRet.TeamRoleData[i].RoleData.QwPlayerId.ToString()
				+ ", slot id : " + oRet.TeamRoleData[i].DwSlotId.ToString()
				+ ", serverid : " + oRet.TeamRoleData[i].DwServerId.ToString() + "\n";
		}

		SampleDebuger.Log(szContent);
	}

	public void OnLoginAckPlayerEnterTeam(byte[] pBuf)
	{
		GameProto.LoginAckPlayerEnterTeam oRet = GameProto.LoginAckPlayerEnterTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerEnterTeam error parse");
			return;
		}

		SampleDebuger.Log("OnLoginAckPlayerEnterTeam result : " + oRet.DwResult.ToString());
	}

	public void OnLoginNotifyPlayerRefuseEnterTeam(byte[] pBuf)
	{
		GameProto.LoginNotifyPlayerRefuseEnterTeam oRet = GameProto.LoginNotifyPlayerRefuseEnterTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginNotifyPlayerRefuseEnterTeam error parse");
			return;
		}

		SampleDebuger.LogError("OnLoginNotifyPlayerRefuseEnterTeam reason : " + oRet.SzReason);
	}

	private void OnLoginAckPlayerLeaveTeam(byte[] pBuf)
	{
		GameProto.LoginAckPlayerLeaveTeam oRet = GameProto.LoginAckPlayerLeaveTeam.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnLoginAckPlayerLeaveTeam error parse");
			return;
		}

		SampleDebuger.Log("OnLoginAckPlayerLeaveTeam result : " + oRet.DwResult.ToString());
	}

	public void OnRoleData(string szData)
	{
		Debug.Log(szData);
		RoleDataRet oData = JsonUtility.FromJson<RoleDataRet>(szData);

		GameProto.PlayerRequestLogin oTest = new GameProto.PlayerRequestLogin();
		oTest.SzToken = oData.token;
		oTest.QwPlayerId = oData.data.id;
		oTest.SzAvatar = oData.data.avatar;
		oTest.SzNickName = oData.data.nick_name;
		oTest.DwSex = oData.data.sex;
		oTest.DwBalance = oData.data.balance;

		byte[] pData = new byte[2048];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 2048);
		pStream.WriteString("GameProto.PlayerRequestLogin");
		byte[] pProto = new byte[oTest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 2048 - pStream.GetLeftLen());

		if (!string.IsNullOrEmpty(oData.game_ip))
		{
			GameInstance.Instance().SetGameIp(oData.game_ip);
			GameInstance.Instance().SetGamePort((ushort)oData.game_port);
		}
	}

	public void IntoInviteTeam(object pParam)
	{
		GameProto.LoginNotifyPlayerInviteTeam oInvite = pParam as GameProto.LoginNotifyPlayerInviteTeam;
		if (oInvite == null)
		{
			SampleDebuger.LogError("IntoInviteTeam parse error!!!!");
			return;
		}
		GameProto.PlayerRequestLoginEnterTeam oRequest = new GameProto.PlayerRequestLoginEnterTeam();
		oRequest.DwTeamServerId = oInvite.DwTeamServerId;
		oRequest.QwTeamId = oInvite.QwTeamId;

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginEnterTeam");
		byte[] pProto = new byte[oRequest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oRequest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void NotIntoInviteTeam(object pParam)
	{
		GameProto.LoginNotifyPlayerInviteTeam oInvite = pParam as GameProto.LoginNotifyPlayerInviteTeam;
		if (oInvite == null)
		{
			SampleDebuger.LogError("NotIntoInviteTeam parse error!!!!");
			return;
		}

		GameProto.PlayerRequestLoginRefuseEnterTeam oRequest = new GameProto.PlayerRequestLoginRefuseEnterTeam();
		oRequest.QwPlayerId = oInvite.QwPlayerId;
		oRequest.SzReason = "refused!!!";

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginRefuseEnterTeam");
		byte[] pProto = new byte[oRequest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oRequest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void OnDestroy()
	{
		if (m_pSession != null)
		{
			m_pSession.m_pfOnConnect.Remove(OnConnect);
			m_pSession.m_pfOnError.Remove(OnError);
			m_pSession.m_pfOnClose.Remove(OnClose);

			m_pSession.UnRegistMessage("GameProto.LoginAckPlayerServerId");
			m_pSession.UnRegistMessage("GameProto.LoginAckPlayerLoginResult");
			m_pSession.UnRegistMessage("GameProto.LoginAckPlayerMakeTeam");
			m_pSession.UnRegistMessage("GameProto.LoginAckPlayerGameStart");
		}
		H5Manager.Instance().GetLoginSessionResetCallBack().Remove(OnLoginSessionReset);
	}

	//public GameObject m_oRoleList;
	public SessionObject m_pSession;
}
