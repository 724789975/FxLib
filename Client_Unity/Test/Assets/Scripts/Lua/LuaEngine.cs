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
		m_pLuaEnv.DoString("require 'main'");
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
		string szFilePath = "";
#if UNITY_EDITOR
		szFilePath = Globals.streamingPath + "/Lua/" + szFileName;
#else
		szFilePath = Globals.persistenPath + "/Lua/" + szFileName;
#endif

		string ext = System.IO.Path.GetExtension(szFilePath).ToLower();
		if (!ext.Equals("lua"))
		{
			szFilePath += ".lua";
		}

		if (!System.IO.File.Exists(szFilePath))
		{
			SampleDebuger.LogColorAquamarine("can't find file " + szFilePath);
			return null;
		}

		SampleDebuger.LogColorAquamarine(szFilePath);

		return System.IO.File.ReadAllBytes(szFilePath);
		//return System.Text.ASCIIEncoding.UTF8.GetBytes(System.IO.File.ReadAllText(szFilePath));
	}

	public LuaEnv GetLuaEnv() { return m_pLuaEnv; }

	LuaEnv m_pLuaEnv;
}

