using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput : MonoBehaviour
{
	void Awake()
	{
#if !UNITY_EDITOR && UNITY_WEBGL
		gameObject.SetActive(false);
#endif
	}

	// Use this for initialization
	void Start ()
	{
		m_inputName.text = PlayerPrefs.GetString(GameConstant.g_szName);
		m_inputPlatform.text = PlayerPrefs.GetString(GameConstant.g_szPlatform);
		m_inputOpenId.text = PlayerPrefs.GetString(GameConstant.g_szOpenId);

		PlayerData.Instance().SetName(m_inputName.text);
		PlayerData.Instance().SetPlatform(m_inputPlatform.text);
		PlayerData.Instance().SetOpenId(m_inputOpenId.text);
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void Confirm()
	{
		if (m_inputName.text.Length > 0)
		{
			PlayerData.Instance().SetName(m_inputName.text);
			PlayerPrefs.SetString(GameConstant.g_szName, m_inputName.text);
		}
		if (m_inputPlatform.text.Length > 0)
		{
			PlayerData.Instance().SetPlatform(m_inputPlatform.text);
			PlayerPrefs.SetString(GameConstant.g_szPlatform, m_inputPlatform.text);
		}
		if (m_inputOpenId.text.Length > 0)
		{
			PlayerData.Instance().SetOpenId(m_inputOpenId.text);
			PlayerPrefs.SetString(GameConstant.g_szOpenId, m_inputOpenId.text);
		}
	}

	public UnityEngine.UI.InputField m_inputName;
	public UnityEngine.UI.InputField m_inputPlatform;
	public UnityEngine.UI.InputField m_inputOpenId;
}

