using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
class ServerInfo
{
    public ushort login_port = 0;
    public string login_ip = "";
    public string url_host = "";
}
[System.Serializable]
class ServerListInfo
{
    public List<ServerInfo> server_infos = new List<ServerInfo>();
}

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
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

    public void OnServerInfo(string szData)
    {
        Debug.Log(szData);
        ServerListInfo oServerList = JsonUtility.FromJson<ServerListInfo>(szData);

        if (oServerList.server_infos.Count == 0)
        {
            H5Helper.H5AlertString("can't find server list!!!!");
            return;
        }
        ServerInfo oServerInfo = oServerList.server_infos[0];

        m_szLoginIp = oServerInfo.login_ip;
        m_wLoginPort = oServerInfo.login_port;
        m_szUrlHost = oServerInfo.url_host;

        GameStart();
    }
    public void GameStart()
    {
        H5Manager.Instance().ConnectLogin();
    }

	public GamePlayType m_eGamePlayType = GamePlayType.GamePlayType_NONE;
	public GamePlayType proGamePlayType
	{
		get { return m_eGamePlayType; }
		set { m_eGamePlayType = value; }
	}

    public ushort proLoginPort {get { return m_wLoginPort; } }
    public string proLoginIp { get { return m_szLoginIp; } }
    public string proUrlHost { get { return m_szUrlHost; } }
    public string proGetRoleUri { get { return m_szGetRoleUri; } }

    public ushort m_wLoginPort;
    public string m_szLoginIp;
    public string m_szUrlHost;

    public string m_szGetRoleUri;
}
