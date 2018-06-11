using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameLogic : SingletonObject<GameLogic>
{
	// Use this for initialization
	void Start ()
	{
		
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}
	
	public UnityEngine.UI.Image[] m_arrBlocks = new UnityEngine.UI.Image[22*12];
	public UnityEngine.UI.Image[] m_arrNextBlocks = new UnityEngine.UI.Image[4*4];
	public UnityEngine.UI.Image[] m_arrCurrBlocks = new UnityEngine.UI.Image[4*4];
}

