using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class VersionUpdateInit : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
		m_pEnvCheckInit.LocalVerCheck();
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	public EnvCheckInit m_pEnvCheckInit;
}
