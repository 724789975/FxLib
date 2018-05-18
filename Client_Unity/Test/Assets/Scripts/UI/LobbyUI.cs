using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyUI : SingletonObject<LobbyUI>
{
	// Use this for initialization
	void Start ()
	{
		CreateInstance(this);

		m_buttonMakeTeam.onClick.AddListener(delegate () { LoginControler.Instance().MakeTeam(); });
		m_buttonTeamStart.onClick.AddListener(delegate () { LoginControler.Instance().TeamStart(); });
		m_buttonOnlinePlayers.onClick.AddListener(delegate () { LoginControler.Instance().OnlinePlayers(); });
		m_buttonLeave.onClick.AddListener(delegate () { LoginControler.Instance().LeaveTeam(); });
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public UnityEngine.UI.Button m_buttonMakeTeam;
	public UnityEngine.UI.Button m_buttonTeamStart;
	public UnityEngine.UI.Button m_buttonOnlinePlayers;
	public UnityEngine.UI.Button m_buttonLeave;
}
