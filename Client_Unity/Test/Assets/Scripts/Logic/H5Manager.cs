using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class H5Manager : MonoBehaviour
{
	// Use this for initialization
	void Start () { }
	
	// Update is called once per frame
	void Update () { }

	void Init(ushort wPlayerPort, ushort wServerPort, ushort wSlaveServerPort)
	{
		m_wPlayerPort = wPlayerPort;
		m_wServerPort = wServerPort;
		m_wSlaveServerPort = wSlaveServerPort;
	}

	ushort m_wPlayerPort;
	ushort m_wServerPort;
	ushort m_wSlaveServerPort;
}
