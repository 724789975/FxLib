﻿using System.IO;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class GameObjectConstant
{
	public static string g_szPrefabPath = "Assets/Resources/Prefab/UI/";
    public static string g_szLoginServerList = "LoginServerList.prefab";
	public static string g_szPlayerList = "RoleList.prefab";
	public static string g_szConfirmPanel = "ConfirmPanel.prefab";

	public static string GetABUIPath(string szPath)
	{
#if UNITY_EDITOR
		if (!AssetBundleManager.SimulateAssetBundleInEditor)
		{
			string ext = Path.GetExtension(szPath).ToLower();
			szPath = (g_szPrefabPath + szPath).Replace(ext, "");
			return szPath.ToLower();
		}
		return g_szPrefabPath + szPath;
#else
		string ext = Path.GetExtension(szPath).ToLower();
		szPath = (g_szPrefabPath + szPath).Replace(ext, "");
		return szPath.ToLower();
#endif
	}
	public static string GetABName(string szPath)
	{
		string ext = Path.GetExtension(szPath).ToLower();
		szPath = szPath.Replace(ext, "");
		return szPath;
	}
}

public class GameConstant
{
	public static string g_szPatchUrl = "";

	public static string g_szChoseServerScene = "chose_server";
	public static string g_szGamePrepareScene = "game_prepare";
	public static string g_szGameScene = "gamescene";
	public static string g_szLobbyScene = "lobby";
	public static string g_szLobbyTeamScene = "lobby_team";
	public static string g_szVersionUpdateScene = "version_update";
	public static string g_szSceneBundlePath = "assets/resources/screen/";

	public static string g_szName = "name";
	public static string g_szPlatform = "platform";
	public static string g_szOpenId = "open_id";
}
