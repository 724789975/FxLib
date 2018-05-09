using System.Collections;
using System.Collections.Generic;
using UnityEngine;


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

		LoginServerList pList = LoginServerList.CreateInstance(GameObjectConstant.g_szLoginServerList, m_pUiCanvas);
		pList.SetServerListInfo(oServerList);
	}

	public void SetServerInfo(ServerInfo oServerInfo)
	{
		m_szLoginIp = oServerInfo.login_ip;
		m_wLoginPort = oServerInfo.login_port;
		m_szUrlHost = oServerInfo.url_host;
    }

    public void GameStart()
    {
        H5Manager.Instance().ConnectLogin();
    }

	public void SetPlatform(string szPlatform) { m_szPlatform = szPlatform; }
	public void SetName(string szName) { m_szName = szName; }
	public void SetHeadImage(string szHeadImage) { m_szHeadImage = szHeadImage; }
	public void SetSex(uint dwSex) { m_dwSex = dwSex; }
	public void SetAccessToken(string szAccessToken) { m_szAccessToken = szAccessToken; }
	public void SetExpiresDate(uint dwExpiresDate) { m_dwExpiresDate = dwExpiresDate; }
	public void SetOpenId(string szOpenId) { m_szOpenId = szOpenId; }

	public GamePlayType m_eGamePlayType = GamePlayType.GamePlayType_NONE;
	public GamePlayType proGamePlayType
	{
		get { return m_eGamePlayType; }
	}

    public ushort proLoginPort {get { return m_wLoginPort; } }
    public string proLoginIp { get { return m_szLoginIp; } }
    public string proUrlHost { get { return m_szUrlHost; } }
    public string proGetRoleUri { get { return m_szGetRoleUri; } }

	public string proPlatform { get { return m_szPlatform; } }
	public string proName { get { return m_szName; } }
	public string proHeadImage { get { return m_szHeadImage; } }
	public uint proSex { get { return m_dwSex; } }
	public string proAccessToken { get { return m_szAccessToken; } }
	public uint proExpiresDate { get { return m_dwExpiresDate; } }
	public string proOpenId { get { return m_szOpenId; } }

	public ushort m_wLoginPort;
    public string m_szLoginIp;
    public string m_szUrlHost;

    public string m_szGetRoleUri;

	//public GameObject m_pServerList;
	public Transform m_pUiCanvas;

	[Header("Platform Info")]
	public string m_szPlatform = "";
	public string m_szName = "";
	public string m_szHeadImage = "";
	public uint m_dwSex = 0;
	public string m_szAccessToken = "";
	public uint m_dwExpiresDate = 0;
	public string m_szOpenId = "";

}
