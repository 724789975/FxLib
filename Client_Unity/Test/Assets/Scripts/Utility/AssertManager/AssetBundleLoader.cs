﻿using UnityEngine;
using System;
using System.Collections;
using System.IO;

#if UNITY_EDITOR
using UnityEditor;
#endif


public class AssetBundleLoader : SingletonObject<AssetBundleLoader>
{
	void Awake()
	{
		DontDestroyOnLoad(this);
		CreateInstance(this);
	}

	IEnumerator Start()
	{
		yield return new WaitForSeconds(2.0f);
		yield return StartCoroutine(Initialize());
	}

	public IEnumerator Initialize()
	{
		SampleDebuger.Log("bundle dir : " + GetBundleDirName());
		var request = AssetBundleManager.Initialize(GetBundleDirName());
		UIProgressBar.SetpAssetBundleLoadOperation(request);
        if (request != null)
		{
			SampleDebuger.Log("begin loading manifest");
			yield return StartCoroutine(request);
		}
		yield return new WaitForSeconds(0.5f);

#if UNITY_WEBGL
		WWW www = new WWW(Globals.wwwStreamingPath + "/version.txt");
		yield return www;
		VersionManager.Instance().proCurVersion = www.text.Trim();
		LoadLevelAsset(GameConstant.g_szChoseServerScene);
#else
		LoadLevelAsset(GameConstant.g_szVersionUpdateScene);
#endif
	}

	public IEnumerator Reload()
	{
		var request = AssetBundleManager.ReloadManifest(GetBundleDirName());
		if (request != null)
		{
			SampleDebuger.Log("reloading manifest");
			yield return StartCoroutine(request);
		}
	}

	IEnumerator CheckVersion()
	{
		string path = Globals.wwwPersistenPath + "/version.txt";
		WWW www = new WWW(path);
		yield return www;
		string oldVersionStr = "0.0.0";
		if (www.error == null)
		{
			oldVersionStr = www.text.Trim();
		}
		Version oldVersion = new Version(oldVersionStr);
		www = new WWW(Globals.wwwStreamingPath + "/version.txt");
		yield return www;
		string curVersionStr = www.text.Trim();
		Version curVersion = new Version(curVersionStr);
		SampleDebuger.Log("old version : " + oldVersion.proCurVersion + ", cur version : " + curVersion.proCurVersion);
		if (oldVersion.IsLower(curVersion))
		{
			DeleteUpdateBundle();
		}
	}

	public string GetBundleUrl(string fileName)
	{
		string szUrl = "";

#if UNITY_EDITOR
		szUrl = Application.dataPath + "/../AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		//szUrl = m_szAssetUrl + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		//return Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
#elif UNITY_WEBGL
		string szPrefix = "";
		if (m_szAssetUrl.Length != 0)
		{
			szPrefix = m_szAssetUrl;
		}
		else
		{
			szPrefix = Application.streamingAssetsPath;
		}
        szUrl = szPrefix + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName + "?" + VersionManager.Instance().GetVersionUrl();
#else
		szUrl = Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		string szUpdatePath = Application.persistentDataPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		if (File.Exists(szUpdatePath))
		{
			szUrl = szUpdatePath;
		}
#endif
		SampleDebuger.LogGreen("bundle url : " + szUrl);
		return szUrl;
	}

	public void DeleteUpdateBundle()
	{
		if (Directory.Exists(Application.persistentDataPath + "/AssetBundles"))
		{
			Directory.Delete(Application.persistentDataPath + "/AssetBundles", true);
		}
	}

	private static string GetBundleDirName()
	{
#if UNITY_EDITOR
		switch (EditorUserBuildSettings.activeBuildTarget)
		{
			case BuildTarget.Android:
				return "Android";
			case BuildTarget.iOS:
				return "iOS";
			case BuildTarget.StandaloneWindows:
			case BuildTarget.StandaloneWindows64:
				return "Windows";
			case BuildTarget.WebGL:
				return "WebGL";
			default:
				return null;
		}
#else
        switch (Application.platform)
        {
            case RuntimePlatform.Android:
                return "Android";
            case RuntimePlatform.IPhonePlayer:
                return "iOS";
            case RuntimePlatform.WindowsPlayer:
                return "Windows";
			case RuntimePlatform.WebGLPlayer:
				return "WebGL";
            default:
                return null;
        }
#endif
	}

	public void LoadAsset(string assetBundleName, string assetName, Action<UnityEngine.Object> fn)
	{
		StartCoroutine(OnLoadAsset(assetBundleName, assetName, fn));
	}

	public IEnumerator OnLoadAsset(string assetBundleName, string assetName, Action<UnityEngine.Object> fn)
	{
		AssetBundleLoadAssetOperation request = AssetBundleManager.LoadAssetAsync(assetBundleName, assetName, typeof(UnityEngine.Object));
		if (request == null)
			yield break;
		yield return StartCoroutine(request);

		UnityEngine.Object obj = request.GetAsset<UnityEngine.Object>();
		if (fn != null) fn(obj);
	}

	public void LoadAllAsset(string assetBundleName, string assetName, Action<UnityEngine.Object[]> fn)
	{
		StartCoroutine(OnLoadAllAsset(assetBundleName, assetName, fn));
	}

	public IEnumerator OnLoadAllAsset(string assetBundleName, string assetName, Action<UnityEngine.Object[]> fn)
	{
		AssetBundleLoadAssetOperation request = AssetBundleManager.LoadAssetAsync(assetBundleName, assetName, typeof(UnityEngine.Object), false);
		if (request == null)
			yield break;
		yield return StartCoroutine(request);

		UnityEngine.Object[] obj = request.GetAllAsset<UnityEngine.Object>();
		//Debug.Log(assetName + (obj == null ? " isn't" : " is") + " loaded successfully at frame " + Time.frameCount);
		if (fn != null) fn(obj);
	}

	public void LoadLevelAsset(string name, Action fn = null)
	{
		string bundle = GameConstant.g_szSceneBundlePath + name;
		StartCoroutine(LoadLevel(bundle.ToLower(), name, fn));
	}

	protected IEnumerator LoadLevel(string assetBundleName, string levelName, Action fn)
	{
		// Debug.Log("Start to load scene " + levelName + " at frame " + Time.frameCount);
		// Load level from assetBundle.
		AssetBundleLoadBaseOperation request = AssetBundleManager.LoadLevelAsync(assetBundleName, levelName, false);
		if (request != null)
		{
			yield return StartCoroutine(request);
		}

		if (fn != null)
		{
			fn();
		}
	}

	void OnDestory()
	{
	}

#if UNITY_WEBGL
	[Tooltip("通过这个url重定义streamingAssetsPath 如果不填 代表是使用系统自带的 只在webgl浏览器下起作用 编辑器无效")]
	[SerializeField]
	string m_szAssetUrl = "";
#endif
}