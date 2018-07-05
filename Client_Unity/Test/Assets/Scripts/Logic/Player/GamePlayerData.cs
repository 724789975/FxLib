using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GamePlayerData : MonoBehaviour
{
	// Use this for initialization
	void Start()
	{
		GameProto.RoleData pData = GameData.Instance().GetRoleData(GameData.Instance().GetSlotPlayer(m_dwSlotId));
		if (pData == null)
		{
			return;
		}
		SetPlayerId(pData.QwPlayerId);
		SetName(pData.SzNickName);
		SetHeadImage(pData.SzAvatar);
		SetSex(pData.DwSex);
	}

	// Update is called once per frame
	void Update()
	{
	}

	public void ShowPlayer()
	{
		GameLogic.Instance().SetTetrisData(TetrisDataManager.Instance().GetTetrisData(proPlayerId), proPlayerId);
    }

	public void SetPlayerId(UInt64 qwUserId) { m_qwPlayerId = qwUserId; }
	public void SetName(string szName) { m_szName = szName; }
	public void SetSex(uint dwSex) { m_dwSex = dwSex; }

	public void SetHeadImage(string szHeadImage)
	{
		m_szHeadImage = szHeadImage;
		StartCoroutine(H5Helper.SendGet(m_szHeadImage, delegate (Texture2D tex)
			{
				m_imageHead.texture = tex;
			})
		);
	}

	public UInt64 proPlayerId { get { return m_qwPlayerId; } }
	public string proName { get { return m_szName; } }
	public string proHeadImage { get { return m_szHeadImage; } }
	public uint proSex { get { return m_dwSex; } }

	public UInt32 m_dwSlotId = 0xFFFFFFFF;
	public UInt64 m_qwPlayerId = 0;
	public UInt32 m_dwSex = 0;

	public string m_szHeadImage = "";
	public string m_szName = "";

	public UnityEngine.UI.RawImage m_imageHead;
	public UnityEngine.UI.Image m_imageHeadFrame;
	public UnityEngine.UI.Text m_textName;
}
