using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyUI : SingletonObject<LobbyUI>
{
	void Awake()
	{
		CreateInstance(this);
	}
	// Use this for initialization
	void Start ()
	{
		m_buttonMakeTeam.onClick.AddListener(delegate () { LoginControler.Instance().MakeTeam(); });
		m_buttonTeamStart.onClick.AddListener(delegate () { LoginControler.Instance().TeamStart(); });
		m_buttonOnlinePlayers.onClick.AddListener(delegate () { LoginControler.Instance().OnlinePlayers(); });
		m_buttonLeave.onClick.AddListener(delegate () { LoginControler.Instance().LeaveTeam(); });

		m_textName.text = PlayerData.Instance().proName;
		StartCoroutine(H5Helper.SendGet(PlayerData.Instance().proHeadImage, delegate (Texture2D tex)
			{
				m_imgHead.texture = tex;
			})
		);
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	[SerializeField]
	UnityEngine.UI.Button m_buttonMakeTeam = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonTeamStart = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonOnlinePlayers = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonLeave = null;
	[SerializeField]
	UnityEngine.UI.RawImage m_imgHead = null;
	[SerializeField]
	UnityEngine.UI.Image m_imgFrame = null;
	[SerializeField]
	UnityEngine.UI.Text m_textName = null;
}
