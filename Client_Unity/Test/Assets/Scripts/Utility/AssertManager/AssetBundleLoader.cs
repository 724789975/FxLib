using UnityEngine;
using System;
using System.Collections;
using System.IO;

#if UNITY_EDITOR
using UnityEditor;
#endif


public class AssetBundleLoader : SingletonObject<AssetBundleLoader>
{
	public string[] preloadBundles;
	const string kAssetBundlesPath = "/AssetBundles/";

	//开始场景ab包配置
	[SerializeField]
	public string sceneAssetBundle;

	[SerializeField]
	public string sceneName;

	private string sceneBundlePath = "assets/resources/screen/";
	private bool firstLoaded = false;
	private bool isResPreloaded = false;

	void Awake()
	{
		DontDestroyOnLoad(this);
		CreateInstance(this);
		Globals.Init();
	}

	IEnumerator Start()
	{
		yield return new WaitForSeconds(2.0f);
		yield return StartCoroutine(Initialize());
	}

	public IEnumerator Initialize()
	{
		SampleDebuger.Log(GetBundleDirName());
		var request = AssetBundleManager.Initialize(GetBundleDirName());
		UIProgressBar.SetpAssetBundleLoadOperation(request);
        if (request != null)
		{
			SampleDebuger.Log("begin loading manifest");
			yield return StartCoroutine(request);
		}
		yield return new WaitForSeconds(0.5f);
		yield return StartCoroutine(CheckVersion());

		if (!firstLoaded && !string.IsNullOrEmpty(sceneName))
		{
			LoadLevelAsset(sceneName);
			firstLoaded = true;
		}

		LoadLevelAsset(GameConstant.g_szChoseServerScene);
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
		SampleDebuger.Log("old version : " + oldVersion.curVersion + ", cur version : " + curVersion.curVersion);
		if (oldVersion.IsLower(curVersion))
		{
			deleteUpdateBundle();
		}
	}

	public string getBundleUrl(string fileName)
	{
#if UNITY_EDITOR
		return Application.dataPath + "/../AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		//return Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
#else
            string url = Application.streamingAssetsPath + "/AssetBundles/" + SysUtil.GetPlatformName() + "/" + fileName;
		  
            return url;
#endif
	}

	public void deleteUpdateBundle()
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
		string bundle = sceneBundlePath + name;
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