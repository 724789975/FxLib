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
		DontDestroyOnLoad(this);
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
		SampleDebuger.Log(szUrl);
		Uri uri = new Uri(szUrl);
		
		string szBaseUrl;
		NameValueCollection nvcParam;
		ParseUrl(szUrl, out szBaseUrl, out nvcParam);

		PlayerData.Instance().SetPlatform(nvcParam.Get("platform"));
		PlayerData.Instance().SetName(nvcParam.Get("name"));
		PlayerData.Instance().SetHeadImage(nvcParam.Get("head_img"));
		PlayerData.Instance().SetAccessToken(nvcParam.Get("access_token"));
		PlayerData.Instance().SetOpenId(nvcParam.Get("openid"));

		string szSex = nvcParam.Get("sex");
		string szExpiresDate = nvcParam.Get("expires_date");
		uint dwSex = 0;
		uint dwExpiresDate = 0;
        uint.TryParse(szSex, out dwSex);
		uint.TryParse(szExpiresDate, out dwExpiresDate);

		PlayerData.Instance().SetSex(dwSex);
		PlayerData.Instance().SetExpiresDate(dwExpiresDate);
	}

	public void ConnectLogin()
	{
        if (m_pLoginSession != null)
        {
            m_pLoginSession.Close();
        }

        m_pLoginSession = gameObject.AddComponent<SessionObject>();
        foreach (var item in m_setLoginSessionResetCallBack)
        {
            item(m_pLoginSession);
        }
		m_pLoginSession.InitSession(SessionObject.SessionType.SessionType_WebSocket,
			GameInstance.Instance().proLoginIp, GameInstance.Instance().proLoginPort);
	}

	public void ConnectGame(string szGameIp, ushort wPort)
	{
		if (m_pGameSession != null)
		{
			m_pGameSession.Close();
		}

		m_pGameSession = gameObject.AddComponent<SessionObject>();
		foreach (var item in m_setGameSessionResetCallBack)
		{
			item(m_pGameSession);
		}
		m_pGameSession.InitSession(SessionObject.SessionType.SessionType_WebSocket, szGameIp, wPort);
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

			//JSTest. Instance().txtRequestUrl = JSTest. Instance().txtRequestUrl + "\n" + strParam + "  " + strValue;
		}
	}

	public SessionObject GetLoginSession()
	{
		return m_pLoginSession;
	}

	public SessionObject GetGameSession()
	{
		return m_pGameSession;
	}

	public HashSet<Action<SessionObject> > GetLoginSessionResetCallBack() { return m_setLoginSessionResetCallBack; }
	public HashSet<Action<SessionObject> > GetGameSessionResetCallBack() { return m_setGameSessionResetCallBack; }

	[SerializeField]
	HashSet<Action<SessionObject> > m_setLoginSessionResetCallBack = new HashSet<Action<SessionObject>>();
	[SerializeField]
	SessionObject m_pLoginSession = null;

	[SerializeField]
	HashSet<Action<SessionObject> > m_setGameSessionResetCallBack = new HashSet<Action<SessionObject> >();
	[SerializeField]
	SessionObject m_pGameSession = null;
}
