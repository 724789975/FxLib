using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyTeamPlayer : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
		m_buttonInvite.onClick.AddListener(delegate () { LoginControler.Instance().OnlinePlayers(); });
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void Init(GameProto.RoleData oData)
	{
		m_qwPlayerId = oData.QwPlayerId;
		m_buttonKick.gameObject.SetActive(true);
		m_textName.text = oData.SzNickName;
		m_buttonInvite.gameObject.SetActive(false);
		StartCoroutine(H5Helper.SendGet(oData.SzAvatar, delegate (Texture2D tex)
			{
				m_imgHead.texture = tex;
				m_imgHead.gameObject.SetActive(true);
			})
		);
	}

	public void Init()
	{
		m_qwPlayerId = 0;
		m_textName.text = "Invite";
		m_buttonInvite.gameObject.SetActive(true);
		m_buttonKick.gameObject.SetActive(false);
		m_imgHead.gameObject.SetActive(false);
	}

	[SerializeField]
	uint m_dwSlotId;
	[SerializeField]
	System.UInt64 m_qwPlayerId;

	[SerializeField]
	UnityEngine.UI.RawImage m_imgHead = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonKick = null;
	[SerializeField]
	UnityEngine.UI.Text m_textName = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonInvite = null;
}

