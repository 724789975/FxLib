using System.Collections;
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
        StartCoroutine(H5Helper.SendGet("http://quchifan.wang/portal/index.php/api/server_list/index", OnServerInfo));
        //StartCoroutine(H5Helper.SendGet("http://127.0.0.1/portal/index.php/api/server_list/index", OnServerInfo));
	}
	
	// Update is called once per frame
	void Update ()
	{
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
        ServerInfo oServerInfo = oServerList.server_infos[0];

		AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szLoginServerList), GameObjectConstant.GetABName(GameObjectConstant.g_szLoginServerList), delegate (UnityEngine.Object ob)
			{
				LoginServerList pList = LoginServerList.CreateInstance(ob, m_pUiCanvas);
				pList.SetServerListInfo(oServerList);
			}
		);
	}

    public void GameStart()
    {
        H5Manager.Instance().ConnectLogin();
    }

	public GamePlayType m_eGamePlayType = GamePlayType.GamePlayType_NONE;
	public GamePlayType proGamePlayType
	{
		get { return m_eGamePlayType; }
	}

	public Transform m_pUiCanvas;
	public PlayerData m_PlayerData = PlayerData.Instance();
}
