using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MainCanvas : SingletonObject<MainCanvas>
{

	// Use this for initialization
	void Start ()
	{
		CreateInstance(this);	
	}
	
	// Update is called once per frame
	void Update ()
	{
	}
}
