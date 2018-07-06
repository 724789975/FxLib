using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ConfirmPanel : MonoBehaviour
{
	// Use this for initialization
	void Start()
	{

	}

	// Update is called once per frame
	void Update()
	{

	}

	public void Init(string szContnet, System.Action<object> pfConfirmFunc, object pParam1, System.Action<object> pfCancleFunc, object pParam2)
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

	public void Init(string szContnet, string szConfirm, string szCancle, System.Action<object> pfConfirmFunc, object pParam1, System.Action<object> pfCancleFunc, object pParam2)
	{
		m_pfConfirmFunc = pfConfirmFunc;
		m_pParam1 = pParam1;
		m_pfCancleFunc = pfCancleFunc;
		m_pParam2 = pParam2;
		m_textContent.text = szContnet;
		m_textConfirm.text = szConfirm;
		m_textCancle.text = szCancle;
		if (m_pfConfirmFunc != null)
		{
			m_buttonConfirm.onClick.AddListener(delegate () { m_pfConfirmFunc(m_pParam1); Destroy(gameObject); });
		}
		if (m_pfCancleFunc != null)
		{
			m_buttonCancle.onClick.AddListener(delegate () { m_pfCancleFunc(m_pParam2); Destroy(gameObject); });
		}
	}

	[SerializeField]
	UnityEngine.UI.Button m_buttonConfirm = null;
	[SerializeField]
	UnityEngine.UI.Text m_textConfirm = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonCancle = null;
	[SerializeField]
	UnityEngine.UI.Text m_textCancle = null;
	[SerializeField]
	UnityEngine.UI.Text m_textContent = null;
	[SerializeField]
	System.Action<object> m_pfConfirmFunc;
	[SerializeField]
	object m_pParam1;
	[SerializeField]
	System.Action<object> m_pfCancleFunc;
	[SerializeField]
	object m_pParam2;
}
