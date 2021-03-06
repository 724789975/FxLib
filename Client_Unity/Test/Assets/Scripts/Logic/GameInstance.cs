﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;


public class GameInstance: SingletonObject<GameInstance>
{
	public enum GamePlayType
	{
		GamePlayType_NONE = 0,
		GamePlayType_Server = 1,
		GamePlayType_Slave = 1 << 1,
		GamePlayType_Player = 1 << 2,

		SLAVE_PLAYER = GamePlayType_Slave | GamePlayType_Player,

	}

	void Awake()
	{
		CreateInstance(this);
	}

	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}
	void OnDestroy()
	{
	}

	void OnApplicationQuit()
	{
	}

	public void GetServerInfo()
	{
		StartCoroutine(H5Helper.SendGet(m_szServerUrl + m_szServerInfoUri, OnServerInfo));
	}

    public void OnServerInfo(string szData)
    {
		SampleDebuger.Log(szData);
        ServerListInfo oServerList = JsonUtility.FromJson<ServerListInfo>(szData);

        if (oServerList.server_infos.Count == 0)
        {
            SampleDebuger.LogError("can't find server list!!!!");
            return;
        }

		AssetBundleLoader.Instance().LoadAsset(
			GameObjectConstant.GetABUIPath(GameObjectConstant.g_szLoginServerList),
			GameObjectConstant.GetABName(GameObjectConstant.g_szLoginServerList),
			delegate (UnityEngine.Object ob)
			{
				LoginServerList pList = LoginServerList.CreateInstance(ob, MainCanvas.Instance().transform);
				pList.SetServerListInfo(oServerList);
			}
		);
	}

	public GamePlayType m_eGamePlayType = GamePlayType.GamePlayType_NONE;
	public GamePlayType proGamePlayType
	{
		get { return m_eGamePlayType; }
	}

	public PlayerData m_PlayerData = PlayerData.Instance();

	public string proServerUrl { get { return m_szServerUrl; } }
	public string proServerVersion { get { return m_szVersion; } }
	public string proServerVersionContent { get { return m_szVersionContent; } }
	public string proServerMarkPath { get { return m_szMarkPath; } }
	public string proServerPlatformContentPath{ get { return m_szPlatformContentPath; } }

	public ushort proLoginPort { get { return m_wLoginPort; } }
	public string proLoginIp { get { return m_szLoginIp; } }
	public string proUrlHost { get { return m_szUrlHost; } }
	public string proGetRoleUri { get { return m_szGetRoleUri; } }

	public void SetServerInfo(ServerInfo oServerInfo)
	{
		m_szLoginIp = oServerInfo.login_ip;
		m_wLoginPort = oServerInfo.login_port;
		m_szUrlHost = oServerInfo.url_host;
	}

	[Header("Server Config")]
	[SerializeField]
	string m_szServerUrl = "";
	[SerializeField]
	string m_szServerInfoUri = "";

	[SerializeField]
	ushort m_wLoginPort;
	[SerializeField]
	string m_szLoginIp;

	[SerializeField]
	string m_szUrlHost;
	[SerializeField]
	string m_szGetRoleUri = "";
	[SerializeField]
	string m_szVersion = "";
	[SerializeField]
	string m_szVersionContent = "";
	[SerializeField]
	string m_szMarkPath = "";
	[SerializeField]
	string m_szPlatformContentPath = "";
}
