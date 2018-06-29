using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ReadyTime : SingletonObject<ReadyTime>
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

	public static void SetReadyTime(uint dwTime)
	{
		if (Instance() == null)
		{
			return;
		}

		Instance().m_textReady.text = string.Format("敌军还有 {0} 秒到达战场...", dwTime.ToString());
	}

	public static void SetGameBegin()
	{
		if (Instance() == null)
		{
			return;
		}
		Instance().StartCoroutine(Instance().OnGameStart());
	}

	public IEnumerator OnGameStart()
	{
		m_textReady.text = "游戏开始!!!";
		yield return new WaitForSeconds(0.5f);
		Destroy(gameObject);
	}

	public UnityEngine.UI.Text m_textReady;
}

