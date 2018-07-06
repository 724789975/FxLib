using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
public class ServerInfo
{
	public uint id = 0;
	public ushort login_port = 0;
	public string login_ip = "";
	public string url_host = "";
}
public class LoginServerInfo : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void GameStart()
	{
		GameInstance.Instance().SetServerInfo(m_pServerInfo);
		H5Manager.Instance().ConnectLogin();
	}

	public void SetServerInfo(ServerInfo oServerInfo)
	{
		m_pServerInfo = oServerInfo;
		m_txtServerInfo.text = oServerInfo.id.ToString() + ":" + oServerInfo.login_ip + ":" + oServerInfo.login_port.ToString() + ":" + oServerInfo.url_host;
	}

	[SerializeField]
	ServerInfo m_pServerInfo = null;
	[SerializeField]
	UnityEngine.UI.Text m_txtServerInfo = null;
	[SerializeField]
	UnityEngine.UI.Button m_pButton = null;
}
