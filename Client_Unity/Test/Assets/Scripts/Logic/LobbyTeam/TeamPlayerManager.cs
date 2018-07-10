using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TeamPlayerManager : SingletonObject<TeamPlayerManager>
{
	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SyncTeamInfo(GameProto.LoginNotifyPlayerTeamInfo oInfo)
	{
		for (int i = 0; i < oInfo.TeamRoleData.Count; i++)
		{
			LobbyTeamPlayer pPlayer = GetPlayerBySlot(oInfo.TeamRoleData[i].DwSlotId);
			pPlayer.Init(oInfo.TeamRoleData[i].RoleData);
		}
	}

	public LobbyTeamPlayer GetPlayerBySlot(uint dwSlotId)
	{
		if (dwSlotId > m_arrRedPlayers.Length)
		{
			return m_arrBluePlayers[dwSlotId - m_arrRedPlayers.Length];
		}
		return m_arrBluePlayers[dwSlotId];
	}

	[SerializeField]
	LobbyTeamPlayer[] m_arrRedPlayers = null;
	LobbyTeamPlayer[] m_arrBluePlayers = null;
}

