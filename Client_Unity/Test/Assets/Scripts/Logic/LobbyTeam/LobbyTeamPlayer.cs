using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyTeamPlayer : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
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

	[UnityEngine.SerializeField]
	uint m_dwSlotId;
	[UnityEngine.SerializeField]
	System.UInt64 m_qwPlayerId;

	[UnityEngine.SerializeField]
	UnityEngine.UI.RawImage m_imgHead = null;
	[UnityEngine.SerializeField]
	UnityEngine.UI.Button m_buttonKick = null;
	[UnityEngine.SerializeField]
	UnityEngine.UI.Text m_textName = null;
	[UnityEngine.SerializeField]
	UnityEngine.UI.Button m_buttonInvite = null;
}

