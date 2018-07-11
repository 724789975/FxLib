using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyTeamPlayer : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
		m_buttonInvite.onClick.AddListener(delegate () { SlotClick(); });
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void Init(GameProto.RoleData oData)
	{
		m_imgHead.gameObject.SetActive(true);
		LobbyTeamPlayer pLeader = TeamPlayerManager.Instance().GetPlayerBySlot(0);
		if (pLeader.m_qwPlayerId != PlayerData.Instance().proPlayerId)
		{
			m_buttonKick.gameObject.SetActive(false);
		}
		else
		{
			if (m_dwSlotId != 0)
			{
				m_buttonKick.gameObject.SetActive(false);
				//todo 先不踢玩家 如果要加这个功能 后面再处理
				m_buttonKick.gameObject.SetActive(true);
			}
			else
			{
				m_buttonKick.gameObject.SetActive(false);
			}
		}
		m_qwPlayerId = oData.QwPlayerId;
		m_textName.text = oData.SzNickName;
		m_buttonInvite.gameObject.SetActive(false);
		StartCoroutine(H5Helper.SendGet(oData.SzAvatar, delegate (Texture2D tex)
			{
				m_imgHead.texture = tex;
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

	void SlotClick()
	{
		if (m_qwPlayerId != 0)
		{
			return;
		}
		LobbyTeamPlayer pLeader = TeamPlayerManager.Instance().GetPlayerBySlot(0);
		if (pLeader.m_qwPlayerId != PlayerData.Instance().proPlayerId)
		{
			//其他人点就是换位置 后面可以加个其他模式
			LoginControler.Instance().ChangeSlot(m_dwSlotId);
		}
		else
		{
			//队长点 就是拉人
			LoginControler.Instance().OnlinePlayers();
		}
	}

	[SerializeField]
	uint m_dwSlotId = 0;
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

