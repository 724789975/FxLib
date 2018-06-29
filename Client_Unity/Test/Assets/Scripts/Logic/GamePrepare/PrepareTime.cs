using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PrepareTime : SingletonObject<PrepareTime>
{
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

	public static void SetPrepareTime(uint dwTime)
	{
		if (Instance() == null)
		{
			return;
		}

		Instance().m_textPrepare.text = string.Format("准备时间剩余 {0} 秒...", dwTime.ToString());
    }

	public UnityEngine.UI.Text m_textPrepare;
}

