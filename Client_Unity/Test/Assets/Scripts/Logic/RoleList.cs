using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RoleList : MonoBehaviour {

	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SetPlayerIds(Google.Protobuf.Collections.RepeatedField<ulong> qwPlayerId)
	{
		for (int i = 0; i < qwPlayerId.Count; ++i)
		{
			GameObject obj = Instantiate(m_pPlayerObj, m_pContentTransform);
			obj.GetComponent<TeamPlayer>().SetPlayerId(qwPlayerId[i]);
		}
	}

	public GameObject m_pPlayerObj;
	public Transform m_pContentTransform;
}
