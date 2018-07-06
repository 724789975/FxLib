﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GamePlayerManager : SingletonObject<GamePlayerManager>
{
	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public GamePlayerData GetPlayerBySlot(int dwSlotId) { return m_arrPlayerDatas[dwSlotId]; }

	[SerializeField]
	GamePlayerData[] m_arrPlayerDatas = null;
}

