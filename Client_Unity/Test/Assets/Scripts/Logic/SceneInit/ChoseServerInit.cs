using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ChoseServerInit : MonoBehaviour
{

	// Use this for initialization
	void Start ()
	{
		GameInstance.Instance().GetServerInfo();
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}
}

