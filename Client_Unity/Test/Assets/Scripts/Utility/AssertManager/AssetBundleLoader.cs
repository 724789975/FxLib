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
		string url = "";
#if UNITY_EDITOR
		url = Application.dataPath + "/../AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		//return Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
#else
		url = Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
#endif
		SampleDebuger.LogGreen("bundle url : " + url);
		return url;
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
}