using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameLogic : SingletonObject<GameLogic>
{
	void Awake()
	{
		for (int i = 0; i < GameData.s_dwColumn * GameData.s_dwRow; ++i)
		{
			m_arrBlockInfos[i / GameData.s_dwColumn, i % GameData.s_dwColumn] = m_arrBlocks[i];
		}
	}

	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}
	
	public UnityEngine.UI.Image[] m_arrBlocks = new UnityEngine.UI.Image[GameData.s_dwRow * GameData.s_dwColumn];
	public UnityEngine.UI.Image[] m_arrNextBlocks = new UnityEngine.UI.Image[GameData.s_dwUnit * GameData.s_dwUnit];
	public UnityEngine.UI.Image[] m_arrCurrBlocks = new UnityEngine.UI.Image[GameData.s_dwUnit * GameData.s_dwUnit];

	UnityEngine.UI.Image[,] m_arrBlockInfos = new UnityEngine.UI.Image[GameData.s_dwRow, GameData.s_dwColumn];

}

