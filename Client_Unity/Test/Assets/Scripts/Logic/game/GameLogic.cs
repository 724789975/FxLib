﻿using System.Collections;
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
		m_oData = TetrisDataManager.Instance().proUserTetrisData;
	}
	
	// Update is called once per frame
	void Update ()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		if (m_oData == null)
		{
			return;
		}

		TetrisDataManager.Instance().proUserTetrisData.Update(Time.deltaTime);

		uint[,] dwBlockInfos = m_oData.GetTetrisInfo();
		if (dwBlockInfos == null)
		{
			return;
		}

		//刷新显示的方块
		Color32 c = new Color32(255, 255, 255, 100);
		for (int i = 0; i < TetrisData.s_dwRow; i++)
		{
			for (int j = 0; j < TetrisData.s_dwColumn; j++)
			{
				uint dwBlock = dwBlockInfos[i, j];
				if (dwBlock != 0)
				{
					c.r = (byte)((dwBlock & 0xFF000000) >> 24);
					c.g = (byte)((dwBlock & 0x00FF0000) >> 16);
					c.b = (byte)((dwBlock & 0x0000FF00) >> 8);
				}
				else
				{
					c.r = 255;
					c.g = 255;
					c.b = 255;
				}
				m_arrBlockInfos[i, j].color = c;
			}
		}

		if (m_oData.m_oNextTetris != null)
		{
			for (int i = 0; i < TetrisData.s_dwUnit; ++i)
			{
				for (int j = 0; j < TetrisData.s_dwUnit; ++j)
				{
					uint dwBlockInfo = TetrisData.s_dwTetrisTable[m_oData.m_oNextTetris.m_dwTetrisShape, m_oData.m_oNextTetris.m_dwTetrisDirect, i, j];
					if (dwBlockInfo != 0)
					{
						c.r = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0xFF000000) >> 24);
						c.g = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0x00FF0000) >> 16);
						c.b = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0x0000FF00) >> 8);
					}
					else
					{
						c.r = 255;
						c.g = 255;
						c.b = 255;
					}
					m_arrNextBlocks[i * TetrisData.s_dwUnit + j].color = c;
				}
			}
		}
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
