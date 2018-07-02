using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GamePlayerManager : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
		
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	GamePlayerData GetPlayerBySlot(int dwSlotId) { return m_arrPlayerDatas[dwSlotId]; }

	public GamePlayerData[] m_arrPlayerDatas;
}

