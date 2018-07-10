using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TeamPlayerManager : SingletonObject<TeamPlayerManager>
{
	// Use this for initialization
	void Start ()
	{
		GameProto.RoleData oData = new GameProto.RoleData();
		oData.DwSex = PlayerData.Instance().proSex;
		oData.QwPlayerId = PlayerData.Instance().proPlayerId;
		oData.SzAvatar = PlayerData.Instance().proHeadImage;
		oData.SzNickName = PlayerData.Instance().proName;

		GetPlayerBySlot(0).Init(oData);
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SyncTeamInfo(GameProto.LoginNotifyPlayerTeamInfo oInfo)
	{
		for (int i = 0; i < m_arrRedPlayers.Length; ++i)
		{
			m_arrRedPlayers[i].Init();
		}
		for (int i = 0; i < m_arrBluePlayers.Length; ++i)
		{
			m_arrBluePlayers[i].Init();
		}
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
		return m_arrRedPlayers[dwSlotId];
	}

	[SerializeField]
	LobbyTeamPlayer[] m_arrRedPlayers = null;
	[SerializeField]
	LobbyTeamPlayer[] m_arrBluePlayers = null;
}

