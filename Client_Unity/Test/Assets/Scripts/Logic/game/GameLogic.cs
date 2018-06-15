using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameLogic : SingletonObject<GameLogic>
{
	void Awake()
	{
		for (int i = 0; i < TetrisData.s_dwColumn * TetrisData.s_dwRow; ++i)
		{
			m_arrBlockInfos[i / TetrisData.s_dwColumn, i % TetrisData.s_dwColumn] = m_arrBlocks[i];
		}
	}

	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
		if (m_oData == null)
		{
			return;
		}
		if (!m_oData.proNeedRefresh == false)
		{
			return;
		}

		//todo 刷新显示的方块
	}

	void SetTetrisData(TetrisData oData)
	{
		m_oData = oData;
	}
	
	public UnityEngine.UI.Image[] m_arrBlocks = new UnityEngine.UI.Image[TetrisData.s_dwRow * TetrisData.s_dwColumn];
	public UnityEngine.UI.Image[] m_arrNextBlocks = new UnityEngine.UI.Image[TetrisData.s_dwUnit * TetrisData.s_dwUnit];
	public UnityEngine.UI.Image[] m_arrCurrBlocks = new UnityEngine.UI.Image[TetrisData.s_dwUnit * TetrisData.s_dwUnit];

	UnityEngine.UI.Image[,] m_arrBlockInfos = new UnityEngine.UI.Image[TetrisData.s_dwRow, TetrisData.s_dwColumn];

	public TetrisData m_oData = null;
}

