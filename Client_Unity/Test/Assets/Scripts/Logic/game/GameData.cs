using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameData
{
	public static readonly uint s_dwColumn = 12;
	public static readonly uint s_dwRow = 22;
	public static readonly uint s_dwUnit = 4;

	// 7种方块的4旋转状态（4位为一行）
	public static readonly ushort[,] s_wTetrisTable =
	{
		{ 0x00F0, 0x2222, 0x00F0, 0x2222 },  // I型
		{ 0x0072, 0x0262, 0x0270, 0x0232 },  // T型
		{ 0x0223, 0x0074, 0x0622, 0x0170 },  // L型
		{ 0x0226, 0x0470, 0x0322, 0x0071 },  // J型
		{ 0x0063, 0x0264, 0x0063, 0x0264 },  // Z型
		{ 0x006C, 0x0462, 0x006C, 0x0462 },  // S型
		{ 0x0660, 0x0660, 0x0660, 0x0660 }   // O型
	};

	//所有的方块 每个元素代表一种颜色
	uint[,] m_dwTetrisPool = new uint[s_dwRow, s_dwColumn];
	//当前方块
	uint[,] m_dwCurrBlock = new uint[s_dwUnit, s_dwUnit];
	//下一个方块
	uint[,] m_dwNextBlock = new uint[s_dwUnit, s_dwUnit];
}

