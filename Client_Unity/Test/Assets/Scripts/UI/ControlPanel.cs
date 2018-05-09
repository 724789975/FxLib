using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ControlPanel : MonoBehaviour
{
	public delegate void ButtonFunc(object param);
	// Use this for initialization
	void Start()
	{

	}

	// Update is called once per frame
	void Update()
	{

	}

	public void Init(string szContnet, ButtonFunc pfConfirmFunc, object pParam1, ButtonFunc pfCancleFunc, object pParam2)
	{
		m_pfConfirmFunc = pfConfirmFunc;
		m_pParam1 = pParam1;
		m_pfCancleFunc = pfCancleFunc;
		m_pParam2 = pParam2;
		m_textContent.text = szContnet;
		if (m_pfConfirmFunc != null)
		{
			m_buttonConfirm.onClick.AddListener(delegate () { m_pfConfirmFunc(m_pParam1); Destroy(gameObject); });
		}
		if (m_pfCancleFunc != null)
		{
			m_buttonCancle.onClick.AddListener(delegate () { m_pfCancleFunc(m_pParam2); Destroy(gameObject); });
		}
	}

	public UnityEngine.UI.Button m_buttonConfirm;
	public UnityEngine.UI.Button m_buttonCancle;
	public UnityEngine.UI.Text m_textContent;
	ButtonFunc m_pfConfirmFunc;
    object m_pParam1;
	ButtonFunc m_pfCancleFunc;
    object m_pParam2;
}
