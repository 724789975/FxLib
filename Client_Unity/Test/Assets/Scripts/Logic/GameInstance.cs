using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameInstance: SingletonObject<GameInstance>
{
	public enum GamePlayType
	{
		GamePlayType_NONE = 0,
		GamePlayType_Server = 1,
		GamePlayType_Slave = 1 << 1,
		GamePlayType_Player = 1 << 2,

		SLAVE_PLAYER = GamePlayType_Slave | GamePlayType_Player,

	}

	void Awake()
	{
		CreateInstance(this);
	}

	// Use this for initialization
	void Start ()
	{
		
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public GamePlayType m_eGamePlayType = GamePlayType.GamePlayType_NONE;
	public GamePlayType proGamePlayType
	{
		get { return m_eGamePlayType; }
		set { m_eGamePlayType = value; }
	}

}
