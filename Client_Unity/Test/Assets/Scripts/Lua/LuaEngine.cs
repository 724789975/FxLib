using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using XLua;

public class LuaEngine : SingletonObject<LuaEngine>
{
	void Awake()
	{
		CreateInstance(this);
		DontDestroyOnLoad(this);
		m_pLuaEnv = new LuaEnv();
	}
	// Use this for initialization
	void Start ()
	{
		
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public LuaEnv GetLuaEnv() { return m_pLuaEnv; }

	LuaEnv m_pLuaEnv;
}

