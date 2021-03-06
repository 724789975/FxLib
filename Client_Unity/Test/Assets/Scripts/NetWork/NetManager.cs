﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class NetManager : SingletonObject<NetManager>
{
	void Awake()
	{
		CreateInstance(this);
#if UNITY_WEBGL && !UNITY_EDITOR
#else
		FxNet.FxNetModule.CreateInstance();
		FxNet.FxNetModule.Instance().Init();
		FxNet.IoThread.CreateInstance();
		FxNet.IoThread.Instance().Init();
		FxNet.IoThread.Instance().Start();
#endif
	}
	// Use this for initialization
	void Start ()
	{
		DontDestroyOnLoad(this);
	}
	
	// Update is called once per frame
	void Update ()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
#else
		FxNet.FxNetModule.Instance().Run();
#endif
	}
}
