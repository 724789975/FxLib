﻿using System;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
public class Tetris
{
	public uint m_dwTetrisShape = 0;
	public uint m_dwTetrisDirect = 0;
	public uint m_dwTetrisColor = 0;

	//坐标位置为左下角
	/// <summary>
	/// 列坐标
	/// </summary>
	public int m_dwPosX = 0;
	/// <summary>
	/// 行坐标
	/// </summary>
	public int m_dwPosY = 0;

	public void Init(GameProto.Tetris oTetris)
	{
		m_dwTetrisShape = oTetris.DwTetrisShape;
		m_dwTetrisDirect = oTetris.DwTetrisDirect;
		m_dwTetrisColor = oTetris.DwTetrisColor;
		m_dwPosX = oTetris.DwPosX;
		m_dwPosY = oTetris.DwPosY;
	}
}

[System.Serializable]
public class GameData : Singleton<GameData>
{
	public void Reset()
	{
		m_eState = GameProto.EGameSceneState.EssNone;
		m_mapSlots.Clear();
		m_mapPlayers.Clear();
	}

	public void SetSlotId(UInt32 dwSlotId, UInt64 qwPlayerId)
	{
		m_mapSlots[dwSlotId] = qwPlayerId;
	}

	public UInt64 GetSlotPlayer(UInt32 dwSlotId)
	{
		if (m_mapSlots.ContainsKey(dwSlotId))
		{
			return m_mapSlots[dwSlotId];
		}
		return 0;
	}

	public void SetRoleDate(UInt64 qwPlayerId, GameProto.RoleData pRoleData)
	{
		m_mapPlayers[qwPlayerId] = pRoleData;
	}

	public GameProto.RoleData GetRoleData(UInt64 qwPlayerId)
	{
		if (m_mapPlayers.ContainsKey(qwPlayerId))
		{
			return m_mapPlayers[qwPlayerId];
		}
		return null;
	}

	public void SetGameSceneState(GameProto.EGameSceneState eState) { m_eState = eState; }
	public GameProto.EGameSceneState proGameSceneState { get{ return m_eState; }}
	public GameProto.EGameSceneState m_eState;

	Dictionary<UInt32, UInt64> m_mapSlots = new Dictionary<UInt32, UInt64>();
	Dictionary<UInt64, GameProto.RoleData> m_mapPlayers = new Dictionary<UInt64, GameProto.RoleData>();
}

[System.Serializable]
public class TeamData : Singleton<TeamData>
{
	public void Reset() { m_pTeamRoleData = null; }
	public void SetTeamRoleData(Google.Protobuf.Collections.RepeatedField<GameProto.TeamRoleData> oData) { m_pTeamRoleData = oData; }
	public Google.Protobuf.Collections.RepeatedField<GameProto.TeamRoleData> proTeamRoleData { get { return m_pTeamRoleData; } }
	Google.Protobuf.Collections.RepeatedField<GameProto.TeamRoleData> m_pTeamRoleData;
}

[System.Serializable]
public class TetrisData
{
	public const uint s_dwColumn = 12;
	public const uint s_dwRow = 22;
	public const uint s_dwUnit = 4;

	//形状的数量
	public const uint s_dwShapeCount = 7;
	//方向的数量
	public const uint s_dwDirectCount = 4;

	public static void SetGameSceneState(GameProto.EGameSceneState eState) { s_eState = eState; }
	public static GameProto.EGameSceneState proState { get { return s_eState; } }
	public static GameProto.EGameSceneState s_eState = GameProto.EGameSceneState.EssNone;

	public static float proSuspendTime
	{
		get
		{
			switch ((GameProto.EGameType)m_oConfig.DwGameType)
			{
				case GameProto.EGameType.GtCommon:
					{
						return m_oConfig.CommonConfig.BaseConfig.FSuspendTime;
					}
				default:
					{
						System.Exception e = new System.Exception();
						e.Source = "error game type";
						throw e;
					}
			}
		}
	}

	public static void InitGameConfig(GameProto.GameNotifyPlayerGameConfig oConfig)
	{
		m_oConfig = oConfig;
	}
	static GameProto.GameNotifyPlayerGameConfig m_oConfig = null;

	// 7种方块的4旋转状态
	public static readonly uint[,,,] s_dwTetrisTable = new uint[7, 4, 7, 4]
	{
		// I型 { 0x00F0, 0x2222, 0x00F0, 0x2222 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFF0, 0x0,     0x0, },	//下边界
				{ 0xFFFFFFF1,  0xFFFFFFF1, 0xFFFFFFF1,     0xFFFFFFF1, },	//左边界
				{ 0xFFFFFFF1,  0xFFFFFFF1,         0xFFFFFFF1,            0xFFFFFFF1, },	//右边界
			},
			{
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0xFFFFFFFF,   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0xFFFFFFF2,   0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0x0,          0xFFFFFFF0,         0x0,     0x0, },	//左边界
				{ 0x0,          0xFFFFFFF3,         0x0,            0x0, },	//右边界
			},
			{
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFFF, 0x0,     0x0, },
				{ 0x0,  0xFFFFFFF0, 0x0,     0x0, },	//下边界
				{ 0xFFFFFFF1,  0xFFFFFFF1, 0xFFFFFFF1,     0xFFFFFFF1, },	//左边界
				{ 0xFFFFFFF1,  0xFFFFFFF1,         0xFFFFFFF1,            0xFFFFFFF1, },	//右边界
			},
			{
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0xFFFFFFFF,   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0x0,          0x0,        0x0,            0x0, },
				{ 0xFFFFFFF2,   0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0x0,          0xFFFFFFF0,         0x0,     0x0, },	//左边界
				{ 0x0,          0xFFFFFFF3,         0x0,            0x0, },	//右边界
			},
		},
		// T型 { 0x0072, 0x0262, 0x0270, 0x0232 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFF1,    0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,     0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF3,     0xFFFFFFF2,         0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF2, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF3,     0xFFFFFFF2,         0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF1,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,      0xFFFFFFF2,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF1,         0x0,     0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF3,         0x0,            0x0, },	//右边界
			},
		},
		//L型 //{ 0x0223, 0x0074, 0x0622, 0x0170 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,            0x0,            0xFFFFFFFF, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF3,   0xFFFFFFF1,         0x0,     0x0, },	//左边界
				{ 0xFFFFFFF3,   0xFFFFFFF3,         0x0,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF1, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,      0xFFFFFFF3,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0xFFFFFFFF, 0x0,            0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0xFFFFFFF3,   0xFFFFFFF1,         0x0,            0x0, },	//右边界
			},
			{
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF3, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0xFFFFFFF1,   0xFFFFFFF2,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,      0xFFFFFFF2,            0x0, },	//右边界
			},
		},
		//J型 { 0x0226, 0x0470, 0x0322, 0x0071 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF1, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF3,      0xFFFFFFF3,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,      0xFFFFFFF2,            0x0, },	//右边界
			},
			{
				{ 0x0,  0xFFFFFFFF, 0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF1,   0xFFFFFFF1,         0x0,     0x0, },	//左边界
				{ 0xFFFFFFF1,   0xFFFFFFF3,         0x0,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF3, },	//下边界
				{ 0xFFFFFFF2,   0xFFFFFFF2,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF3,   0xFFFFFFF2,      0xFFFFFFF2,            0x0, },	//右边界
			},
		},
		//Z型 { 0x0063, 0x0264, 0x0063, 0x0264 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,     0x0,           0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF3,   0xFFFFFFF2,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF3,   0xFFFFFFF3,      0xFFFFFFF2,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF1, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF3,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,     0x0,           0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF2, },	//下边界
				{ 0xFFFFFFF3,   0xFFFFFFF2,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0xFFFFFFF3,   0xFFFFFFF3,      0xFFFFFFF2,            0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF1, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF2,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,         0xFFFFFFF3,         0x0, },	//右边界
			},
		},
		//S型 { 0x006C, 0x0462, 0x006C, 0x0462 },	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0x0,           0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF0, },	//下边界
				{ 0x0,  0xFFFFFFF2,         0xFFFFFFF2,  0xFFFFFFF3, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF3,    0xFFFFFFF3, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0xFFFFFFF2, },	//下边界
				{ 0x0,  0xFFFFFFF2,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF3,         0xFFFFFFF2,         0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,     0x0,           0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,     0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0x0,     0x0,           0xFFFFFFFF, },
				{ 0x0,  0x0,     0xFFFFFFF1, 0xFFFFFFF0, },	//下边界
				{ 0x0,  0xFFFFFFF2,         0xFFFFFFF2,  0xFFFFFFF3, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF3,         0xFFFFFFF3, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0x0,            0xFFFFFFFF, 0xFFFFFFFF, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0xFFFFFFF2, },	//下边界
				{ 0x0,  0xFFFFFFF2,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF3,         0xFFFFFFF2,         0x0, },	//右边界
			},
		},
		//O型 { 0x0660, 0x0660, 0x0660, 0x0660 }	第五行代表最下面的块的位置
		{
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF2,            0x0, },	//右边界
			},{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF2,         0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF2,         0x0, },	//右边界
			},
			{
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
				{ 0x0,  0x0,            0x0,            0x0, },
				{ 0x0,  0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//下边界
				{ 0x0,  0xFFFFFFF1,         0xFFFFFFF1,  0x0, },	//左边界
				{ 0x0,  0xFFFFFFF2,      0xFFFFFFF2,         0x0, },	//右边界
			},
		},
	};

	public void Sync(GameProto.GameNotifyPlayerGameInitTetris oTetris)
	{
		m_dwTetrisPool = new uint[s_dwRow, s_dwColumn];
		m_oCurrentTetris.Init(oTetris.CurrTetris);
		m_oNextTetris.Init(oTetris.NextTetris);
		m_bNeedRefresh = true;
	}

	public void Sync(GameProto.GameNotifyPlayerGameTetrisData oTetris)
	{
		m_dwTetrisPool = new uint[s_dwRow, s_dwColumn];
		for (int i = 0; i < s_dwRow; i++)
		{
			for (int j = 0; j < s_dwColumn; j++)
			{
				m_dwTetrisPool[i, j] = oTetris.DwData[i * (int)s_dwColumn + j];
			}
		}
		m_oCurrentTetris.Init(oTetris.CurrTetris);
		m_oNextTetris.Init(oTetris.NextTetris);
		m_bNeedRefresh = true;
	}

	public void Sync(GameProto.GameNotifyPlayerNextTetris oTetris)
	{
		if (m_oNextTetris == null)
		{
			m_oNextTetris = new Tetris();
		}
		m_oNextTetris.Init(oTetris.NextTetris);
		SampleDebuger.LogGreen(string.Format("next tetris [{0},{1}]", oTetris.NextTetris.DwTetrisShape, oTetris.NextTetris.DwTetrisDirect));
		m_bNeedRefresh = true;
	}

	public uint[,] GetTetrisInfo()
	{
		if (m_bNeedRefresh == false)
		{
			return null;
		}

		uint[,] dwTetrisPool = (uint[,])m_dwTetrisPool.Clone();
		for (int i = 0; i < s_dwUnit; ++i)
		{
			for (int j = 0; j < s_dwUnit; ++j)
			{
				uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, i, j];
				if (dwBlockInfo == 0)
				{
					continue;
				}
				dwTetrisPool[m_oCurrentTetris.m_dwPosY - s_dwUnit + i, m_oCurrentTetris.m_dwPosX + j] = m_oCurrentTetris.m_dwTetrisColor;
			}
		}
		m_bNeedRefresh = false;
		return dwTetrisPool;
	}

	public bool CheckErase(ref int dwRow)
	{
		for (int i = 0; i < s_dwRow; ++i)
		{
			bool bErase = true;
			for (int j = 0; j < s_dwColumn; ++j)
			{
				if (m_dwTetrisPool[i, j] == 0)
				{
					bErase = false;
					break;
				}
			}
			if (bErase)
			{
				dwRow = i;
				return true;
			}
		}

		return false;
	}

	public void EraseTetris(int dwRow)
	{
		for (int i = 0; i < s_dwColumn; ++i)
		{
			m_dwTetrisPool[dwRow, i] = 0;
		}
		for (int i = dwRow; i > 0; --i)
		{
			for (int j = 0; j < s_dwColumn; ++j)
			{
				m_dwTetrisPool[i, j] = m_dwTetrisPool[i - 1, j];
			}
		}
		for (int i = 0; i < s_dwColumn; ++i)
		{
			m_dwTetrisPool[0, i] = 0;
		}
	}

	/// <summary>
	/// 检查下面是不是有块
	/// </summary>
	/// <param name="dwCol">列</param>
	/// <param name="dwRow">行</param>
	/// <returns>下面有块 或者到边界了 返回true</returns>
	public bool CheckTetris(int dwCol, int dwRow)
	{
		if (dwRow >= s_dwRow)
		{
			return true;
		}
		if (dwRow < 0)
		{
			return true;
		}
		if (dwCol >= s_dwColumn)
		{
			return true;
		}
		if (dwCol < 0)
		{
			return true;
		}
		uint dwColor = (m_dwTetrisPool[dwRow, dwCol] & 0xFFFFFF00);
		return dwColor != 0;
	}

	public bool CheckLeftTetris()
	{
		//检查方块左面有没有
		for (int i = 0; i < s_dwUnit; i++)
		{
			uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, 5, i];
			if (dwBlockInfo == 0)
			{
				continue;
			}

			int dwCheckX = m_oCurrentTetris.m_dwPosX + ((int)dwBlockInfo & 0x0000000F) - 1;
			int dwCheckY = m_oCurrentTetris.m_dwPosY - (int)s_dwUnit + i;
			//SampleDebuger.LogRed(string.Format("pos [{0},{1}], check pos [{2},{3}], tetris [{4},{5}]",
			//	m_oCurrentTetris.m_dwPosY.ToString(), m_oCurrentTetris.m_dwPosX.ToString(),
			//	dwCheckY.ToString(), dwCheckX.ToString(), m_oCurrentTetris.m_dwTetrisShape.ToString(), m_oCurrentTetris.m_dwTetrisDirect.ToString()));
			if (CheckTetris(dwCheckX, dwCheckY))
			{
				return true;
			}
		}
		return false;
	}

	public bool CheckRightTetris()
	{
		//检查方块右面有没有
		for (int i = 0; i < s_dwUnit; i++)
		{
			uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, 6, i];
			if (dwBlockInfo == 0)
			{
				continue;
			}

			int dwCheckX = m_oCurrentTetris.m_dwPosX + ((int)dwBlockInfo & 0x0000000F) + 1;
			int dwCheckY = m_oCurrentTetris.m_dwPosY - (int)s_dwUnit + i;
			//SampleDebuger.LogRed(string.Format("pos [{0},{1}], check pos [{2},{3}], tetris [{4},{5}]",
			//	m_oCurrentTetris.m_dwPosY.ToString(), m_oCurrentTetris.m_dwPosX.ToString(),
			//	dwCheckY.ToString(), dwCheckX.ToString(), m_oCurrentTetris.m_dwTetrisShape.ToString(), m_oCurrentTetris.m_dwTetrisDirect.ToString()));
			if (CheckTetris(dwCheckX, dwCheckY))
			{
				return true;
			}
		}
		return false;
	}

	public bool CheckDownTetris()
	{
		//检查方块还能不能继续下降
		for (int i = 0; i < s_dwUnit; i++)
		{
			uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, 4, i];
			if (dwBlockInfo == 0)
			{
				continue;
			}

			int dwCheckY = m_oCurrentTetris.m_dwPosY - ((int)dwBlockInfo & 0x0000000F);
			int dwCheckX = m_oCurrentTetris.m_dwPosX + i;
			//SampleDebuger.LogRed(string.Format("pos [{0},{1}], check pos [{2},{3}], tetris [{4},{5}]",
			//	m_oCurrentTetris.m_dwPosY.ToString(), m_oCurrentTetris.m_dwPosX.ToString(),
			//	dwCheckY.ToString(), dwCheckX.ToString(), m_oCurrentTetris.m_dwTetrisShape.ToString(), m_oCurrentTetris.m_dwTetrisDirect.ToString()));
			if (CheckTetris(dwCheckX, dwCheckY))
			{
				return true;
			}
		}
		return false;
	}

	public virtual bool DownTetris()
	{
		if (!CheckDownTetris())
		{
			//向下移动一格
			m_oCurrentTetris.m_dwPosY += 1;
		}
		else
		{
			SampleDebuger.LogGreen(string.Format("down [{0},{1}]", m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect));
			//固定住 那么就不能往下移动了 换下一个方块
			for (int i = 0; i < s_dwUnit; ++i)
			{
				for (int j = 0; j < s_dwUnit; ++j)
				{
					uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, i, j];
					if (dwBlockInfo == 0)
					{
						continue;
					}
					m_dwTetrisPool[m_oCurrentTetris.m_dwPosY - s_dwUnit + i, m_oCurrentTetris.m_dwPosX + j] = m_oCurrentTetris.m_dwTetrisColor;
				}
			}

			if (m_oCurrentTetris.m_dwPosY <= s_dwUnit)
			{
				OnEnd();
				return true;
			}

			int dwRow = -1;
			while (CheckErase(ref dwRow))
			{
				EraseTetris(dwRow);
			}

			m_oCurrentTetris = m_oNextTetris;
			m_oNextTetris = null;
		}
		m_bNeedRefresh = true;
		return true;
	}

	public virtual bool LeftTetris()
	{
		if (!CheckLeftTetris())
		{
			m_oCurrentTetris.m_dwPosX -= 1;
			m_bNeedRefresh = true;
			return true;
		}
		return false;
	}

	public virtual bool RightTetris()
	{
		if (!CheckRightTetris())
		{
			m_oCurrentTetris.m_dwPosX += 1;
			m_bNeedRefresh = true;
			return true;
		}
		return false;
	}

	public virtual bool LeftRotation()
	{
		uint dwTempDir = (m_oCurrentTetris.m_dwTetrisDirect - 1) % s_dwUnit;
		for (int i = 0; i < s_dwUnit; ++i)
		{
			for (int j = 0; j < s_dwUnit; ++j)
			{
				uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, dwTempDir, i, j];
				if (dwBlockInfo == 0)
				{
					continue;
				}
				if (CheckTetris(m_oCurrentTetris.m_dwPosX + j, m_oCurrentTetris.m_dwPosY + i))
				{
					return false;
				}
			}
		}
		m_oCurrentTetris.m_dwTetrisDirect = dwTempDir;
		m_bNeedRefresh = true;
		return true;
	}

	public virtual bool RightRotation()
	{
		uint dwTempDir = (m_oCurrentTetris.m_dwTetrisDirect + 1) % s_dwUnit;
		for (int i = 0; i < s_dwUnit; ++i)
		{
			for (int j = 0; j < s_dwUnit; ++j)
			{
				uint dwBlockInfo = s_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape, dwTempDir, i, j];
				if (dwBlockInfo == 0)
				{
					continue;
				}
				if (CheckTetris(m_oCurrentTetris.m_dwPosX + j, m_oCurrentTetris.m_dwPosY + i))
				{
					return false;
				}
			}
		}
		m_oCurrentTetris.m_dwTetrisDirect = dwTempDir;
		m_bNeedRefresh = true;
		return true;
	}

	public bool TetrisOperator(GameProto.EMoveDirection eMove)
	{
		switch (eMove)
		{
			case GameProto.EMoveDirection.EmdDown: DownTetris(); return true;
			case GameProto.EMoveDirection.EmdLeft: LeftTetris(); return true;
			case GameProto.EMoveDirection.EmdRight: RightTetris(); return true;
		}
		return false;
	}

	public bool TetrisOperator(GameProto.ERotationDirection eRot)
	{
		switch (eRot)
		{
			case GameProto.ERotationDirection.ErdLeft: LeftRotation(); return true;
			case GameProto.ERotationDirection.ErdRight: RightRotation(); return true;
		}
		return false;
	}

	public virtual void OnEnd() { }

	//所有的方块 每个元素代表一种颜色
	public uint[,] m_dwTetrisPool = new uint[s_dwRow, s_dwColumn];

	public bool proNeedRefresh { get { return m_bNeedRefresh; } }

	public Tetris m_oCurrentTetris = new Tetris();

	public Tetris m_oNextTetris = new Tetris();

	protected bool m_bNeedRefresh = false;
}

public class UserTetrisData : TetrisData
{
	public override bool DownTetris()
	{
		if (m_oNextTetris == null)
		{
			return false;
		}
		if (base.DownTetris())
		{
			GameProto.PlayerRequestMove oRequest = new GameProto.PlayerRequestMove();
			oRequest.EDirection = GameProto.EMoveDirection.EmdDown;
			oRequest.FTick = m_fTick;
			SysUtil.SendMessage(GameControler.Instance().proSession, oRequest, "GameProto.PlayerRequestMove");
			return true;
		}
		return false;
	}

	public override bool LeftTetris()
	{
		if (m_oNextTetris == null)
		{
			return false;
		}
		if (base.LeftTetris())
		{
			GameProto.PlayerRequestMove oRequest = new GameProto.PlayerRequestMove();
			oRequest.EDirection = GameProto.EMoveDirection.EmdLeft;
			oRequest.FTick = m_fTick;
			SysUtil.SendMessage(GameControler.Instance().proSession, oRequest, "GameProto.PlayerRequestMove");
			return true;
		}
		return false;
	}

	public override bool RightTetris()
	{
		if (m_oNextTetris == null)
		{
			return false;
		}
		if (base.RightTetris())
		{
			GameProto.PlayerRequestMove oRequest = new GameProto.PlayerRequestMove();
			oRequest.EDirection = GameProto.EMoveDirection.EmdRight;
			oRequest.FTick = m_fTick;
			SysUtil.SendMessage(GameControler.Instance().proSession, oRequest, "GameProto.PlayerRequestMove");
			return true;
		}
		return false;
	}

	public override bool LeftRotation()
	{
		if (m_oNextTetris == null)
		{
			return false;
		}
		if (base.LeftRotation())
		{
			GameProto.PlayerRequestRotation oRequest = new GameProto.PlayerRequestRotation();
			oRequest.EDirection = GameProto.ERotationDirection.ErdLeft;
			oRequest.FTick = m_fTick;
			SysUtil.SendMessage(GameControler.Instance().proSession, oRequest, "GameProto.PlayerRequestRotation");
			return true;
		}
		return false;
	}

	public override bool RightRotation()
	{
		if (m_oNextTetris == null)
		{
			return false;
		}
		if (base.RightRotation())
		{
			GameProto.PlayerRequestRotation oRequest = new GameProto.PlayerRequestRotation();
			oRequest.EDirection = GameProto.ERotationDirection.ErdRight;
			oRequest.FTick = m_fTick;
			SysUtil.SendMessage(GameControler.Instance().proSession, oRequest, "GameProto.PlayerRequestRotation");
			return true;
		}
		return false;
	}

	public void Update(float fDeltaTime)
	{
		m_fTick += fDeltaTime;
		m_dSuspendTime += fDeltaTime;
		if (m_oNextTetris == null)
		{
			return;
		}
		if (m_dSuspendTime > proSuspendTime)
		{
			m_dSuspendTime = 0;
			DownTetris();
		}
	}

	public override void OnEnd()
	{
		SampleDebuger.LogGreen("player die");
	}

	float m_fTick = 0;
	float m_dSuspendTime = 0;
}

public class TetrisDataManager : Singleton<TetrisDataManager>
{
	public TetrisData GetTetrisData(System.UInt64 qwPlayerId)
	{
		if (!m_mapTetrisDatas.ContainsKey(qwPlayerId))
		{
			return null;
		}
		return m_mapTetrisDatas[qwPlayerId];
	}

	public void SetOwner(System.UInt64 qwPlayerId)
	{
		m_mapTetrisDatas[qwPlayerId] = m_pUserTetrisData;
	}

	public void SetPlayer(System.UInt64 qwPlayerId)
	{
		if (m_mapTetrisDatas.ContainsKey(qwPlayerId))
		{
			return;
		}
		m_mapTetrisDatas[qwPlayerId] = new TetrisData();
	}

	Dictionary<System.UInt64, TetrisData> m_mapTetrisDatas = new Dictionary<System.UInt64, TetrisData>();

	UserTetrisData m_pUserTetrisData = new UserTetrisData();

	public UserTetrisData proUserTetrisData { get { return m_pUserTetrisData; } }

}
