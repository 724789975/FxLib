using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LobbyTeamInit : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
		TeamPlayerManager.Instance().SyncTeamInfo(TeamData.Instance().proTeamRoleData);
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}
}

