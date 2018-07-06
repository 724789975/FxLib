using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RoleList : UiSingleton<RoleList>
{

	// Use this for initialization
	void Start ()
	{
		m_buttonClose.onClick.AddListener(delegate () { Close(); });
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SetPlayerIds(Google.Protobuf.Collections.RepeatedField<ulong> qwPlayerId)
	{
		for (int i = m_pContentTransform.childCount - 1; i >= 0; --i)
		{
			Destroy(m_pContentTransform.GetChild(i).gameObject);
		}
		for (int i = 0; i < qwPlayerId.Count; ++i)
		{
			GameObject obj = Instantiate(m_pPlayerObj, m_pContentTransform);
			obj.GetComponent<TeamPlayer>().SetPlayerId(qwPlayerId[i]);
		}
	}

	public void Close()
	{
		Destroy(gameObject);
	}

	[SerializeField]
	GameObject m_pPlayerObj = null;
	[SerializeField]
	Transform m_pContentTransform = null;
	[SerializeField]
	UnityEngine.UI.Button m_buttonClose = null;
}
