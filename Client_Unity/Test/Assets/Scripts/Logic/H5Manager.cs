using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Collections.Specialized;
using System.Text.RegularExpressions;
using System;

public class H5Manager : SingletonObject<H5Manager>
{
	void Awake()
	{
		CreateInstance(this);
	}
	// Use this for initialization
	void Start ()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		Init(H5Helper.H5GetUrl());
#endif
	}

	// Update is called once per frame
	void Update () { }

	public void Init(string szUrl)
	{
		H5Helper.H5LogStr(szUrl);
		Uri uri = new Uri(szUrl);
		
		string szBaseUrl;
		NameValueCollection nvcParam;
		ParseUrl(szUrl, out szBaseUrl, out nvcParam);

		//string szServerPort = nvcParam.Get("server_port");
		//string szSlaveServerPort = nvcParam.Get("slave_server_port");
		//string szPlayerPort = nvcParam.Get("player_port");
		//string szGamePlayType = nvcParam.Get("game_play_type");
		//H5Helper.H5LogStr("server_port : " + szServerPort + ", slave_server_port : "
		//	+ szSlaveServerPort + ", player_port : " + szPlayerPort);

		//if (szGamePlayType == "player")
		//{
		//	//先连这个 后面再改
		//	ushort.TryParse(szServerPort, out m_wServerPort);
		//	ushort.TryParse(szPlayerPort, out m_wPlayerPort);
		//	m_pServerSession.InitSession(SessionObject.SessionType.SessionType_WebSocket, uri.Host, m_wServerPort);
		//	GameInstance.Instance().proGamePlayType = GameInstance.GamePlayType.GamePlayType_Player;
		//}
		//else if (szGamePlayType == "slave")
		//{
		//	ushort.TryParse(szSlaveServerPort, out m_wSlaveServerPort);
		//	m_pServerSession.InitSession(SessionObject.SessionType.SessionType_WebSocket, uri.Host, m_wServerPort);
		//	GameInstance.Instance().proGamePlayType = GameInstance.GamePlayType.GamePlayType_Slave;
		//}
		//else
		//{
		//	H5Helper.H5AlertString("err game type!!!");
		//}
	}

	public void InitTest()
	{
		m_wPlayerPort = 31002;
		m_pLoginSession.InitSession(SessionObject.SessionType.SessionType_WebSocket, "127.0.0.1", m_wLoginPort);
		////Init("http://127.0.0.1/view/index.html?server_port=" + m_wServerPort + "&slave_server_port=1&player_port=2&game_play_type=player");
		//string szUrl = "http://127.0.0.1/portal/index.php/api/login/sendOauthUserInfo";
		//WWWForm form = new WWWForm();
		//form.AddField("platform", "unity_test");
		//form.AddField("name", "test");
		//form.AddField("head_img", "no pic");
		//form.AddField("sex", "1");
		//form.AddField("access_token", "asdfghjk");
		//form.AddField("expires_date", ((int)Time.time + 3600).ToString());
		//form.AddField("openid", "unity");

		//StartCoroutine(H5Helper.SendPost(szUrl, form, OnRoleData));
	}

	static public void ParseUrl(string strOrgUrl, out string strBaseUrl, out NameValueCollection nvcParams)
	{
		if (strOrgUrl == null)
		{
			throw new ArgumentNullException("strOrgUrl");
		}
		strBaseUrl = "";
		nvcParams = new NameValueCollection();
		if (strOrgUrl == "")
		{
			return;
		}

		int nRequestMarkIndex = strOrgUrl.IndexOf('?');
		if (nRequestMarkIndex == -1)
		{
			strBaseUrl = strOrgUrl;
			return;
		}
		strBaseUrl = strOrgUrl.Substring(0, nRequestMarkIndex);

		if (nRequestMarkIndex == strOrgUrl.Length - 1)
		{
			return;
		}

		string strUrlParams = strOrgUrl.Substring(nRequestMarkIndex + 1);

		// 开始分析参数
		Regex regMatch = new Regex(@"(^|&)?(\w+)=([^&]+)(&|$)?");
		MatchCollection mcParams = regMatch.Matches(strUrlParams);
		foreach (Match mc in mcParams)
		{
			string strParam = mc.Result("$2");
			string strValue = mc.Result("$3");
			nvcParams.Add(strParam, strValue);

			//JSTest.instance.txtRequestUrl = JSTest.instance.txtRequestUrl + "\n" + strParam + "  " + strValue;
		}
	}

	public SessionObject GetLoginSession()
	{
		return m_pLoginSession;
	}

    public void OnRoleData(string szData)
    {
        Debug.Log(szData);
    }

	public ushort m_wPlayerPort = 0;
	public ushort m_wLoginPort = 0;
	public ushort m_wSlaveServerPort = 0;

	public SessionObject m_pLoginSession;
}
