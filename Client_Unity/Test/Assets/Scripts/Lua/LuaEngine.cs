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
		//注 一定不能放到awake中
		m_pLuaEnv.AddLoader(StreamingLoader);
	}
	
	// Update is called once per frame
	void Update ()
	{
		if (m_pLuaEnv != null)
		{
			m_pLuaEnv.Tick();
		}
	}

	void OnDestroy()
	{
		if (m_pLuaEnv != null)
		{
			m_pLuaEnv.Dispose();
		}
	}

	private byte[] StreamingLoader(ref string szFileName)
	{
		SampleDebuger.LogBlue(szFileName);

		//找到指定文件
		string szContent = FileUtil.ReadTextFromFile(szFileName);
		return System.Text.ASCIIEncoding.UTF8.GetBytes(szContent);
	}

	public LuaEnv GetLuaEnv() { return m_pLuaEnv; }

	LuaEnv m_pLuaEnv;
}

