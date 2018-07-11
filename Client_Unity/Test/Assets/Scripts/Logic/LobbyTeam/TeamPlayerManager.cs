using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TeamPlayerManager : SingletonObject<TeamPlayerManager>
{
	void Awake()
	{
		CreateInstance(this);
	}
	// Use this for initialization
	void Start ()
	{
		SyncTeamInfo(TeamData.Instance().proTeamRoleData);
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public static void SyncTeamInfo(GameProto.LoginNotifyPlayerTeamInfo oInfo)
	{
		if (Instance() == null)
		{
			return;
		}
		for (int i = 0; i < Instance().m_arrRedPlayers.Length; ++i)
		{
			Instance().m_arrRedPlayers[i].Init();
		}
		for (int i = 0; i < Instance().m_arrBluePlayers.Length; ++i)
		{
			Instance().m_arrBluePlayers[i].Init();
		}
		for (int i = 0; i < oInfo.TeamRoleData.Count; i++)
		{
			LobbyTeamPlayer pPlayer = Instance().GetPlayerBySlot(oInfo.TeamRoleData[i].DwSlotId);
			pPlayer.Init(oInfo.TeamRoleData[i].RoleData);
		}
	}

	void SyncTeamInfo(Google.Protobuf.Collections.RepeatedField<GameProto.TeamRoleData> oInfo)
	{
		if (oInfo == null)
		{
			return;
		}
		for (int i = 0; i < m_arrRedPlayers.Length; ++i)
		{
			m_arrRedPlayers[i].Init();
		}
		for (int i = 0; i < m_arrBluePlayers.Length; ++i)
		{
			m_arrBluePlayers[i].Init();
		}
		for (int i = 0; i < oInfo.Count; i++)
		{
			LobbyTeamPlayer pPlayer = Instance().GetPlayerBySlot(oInfo[i].DwSlotId);
			pPlayer.Init(oInfo[i].RoleData);
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

