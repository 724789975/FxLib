using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GamePlayerData : MonoBehaviour
{
	// Use this for initialization
	void Start()
	{
	}

	// Update is called once per frame
	void Update()
	{
	}

	public void ShowPlayer()
	{
		GameLogic.Instance().SetTetrisData(TetrisDataManager.Instance().GetTetrisData(m_qwPlayerId), m_qwPlayerId);
    }

	public void SetPlayerId(UInt64 qwUserId) { m_qwPlayerId = qwUserId; }
	public void SetName(string szName) { m_szName = szName; }
	public void SetHeadImage(string szHeadImage) { m_szHeadImage = szHeadImage; }
	public void SetSex(uint dwSex) { m_dwSex = dwSex; }

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
	public UnityEngine.UI.RawImage m_imageHeadFrame;
	public UnityEngine.UI.Text m_textName;
}
