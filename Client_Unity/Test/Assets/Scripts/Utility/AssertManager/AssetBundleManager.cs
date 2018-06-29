using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif
using System.Collections;
using System.Collections.Generic;

// Loaded assetBundle contains the references count which can be used to unload dependent assetBundles automatically.
public class LoadedAssetBundle
{
    public AssetBundle m_AssetBundle;
    public int m_ReferencedCount;


    public LoadedAssetBundle(AssetBundle assetBundle)
    {
        m_AssetBundle = assetBundle;
        m_ReferencedCount = 1;
    }
}

public class LoadingAssetBundle
{
    public WWW mWWW;
    public int m_ReferencedCount;

    public LoadingAssetBundle(WWW www)
    {
        mWWW = www;
        m_ReferencedCount = 1;
    }

    ~LoadingAssetBundle()
    {
        if (mWWW != null)
            mWWW.Dispose();
    }
}

// Class takes care of loading assetBundle and its dependencies automatically, loading variants automatically.
public class AssetBundleManager : MonoBehaviour
{
    //static string m_BaseDownloadingURL = "";
    static string[] m_Variants = { };
    public static AssetBundleManifest m_AssetBundleManifest = null;

#if UNITY_EDITOR
    static int m_SimulateAssetBundleInEditor = -1; // 0: 读取ab包， // -1:读取prefab
    const string kSimulateAssetBundles = "SimulateAssetBundles";
#endif

    static Dictionary<string, LoadedAssetBundle> m_LoadedAssetBundles = new Dictionary<string, LoadedAssetBundle>();
    static List<AssetBundleLoadOperation> m_InProgressOperations = new List<AssetBundleLoadOperation>();
    static Dictionary<string, string[]> m_Dependencies = new Dictionary<string, string[]>();

#if UNITY_WEBGL
	static Dictionary<string, WWW> m_LoadingRequest = new Dictionary<string, WWW>();
#else
	static Dictionary<string, AssetBundleCreateRequest> m_LoadingRequest = new Dictionary<string, AssetBundleCreateRequest>();
#endif

	public List<string> _removeList = new List<string>();

    // Variants which is used to define the active variants.
    public static string[] Variants
    {
        get { return m_Variants; }
        set { m_Variants = value; }
    }

    // AssetBundleManifest object which can be used to load the dependecies and check suitable assetBundle variants.
    public static AssetBundleManifest AssetBundleManifestObject
    {
        set { m_AssetBundleManifest = value; }
    }

    void Awake()
    {
		DontDestroyOnLoad(this);
    }

#if UNITY_EDITOR
    // Flag to indicate if we want to simulate assetBundles in Editor without building them actually.
    public static bool SimulateAssetBundleInEditor
    {
        get
        {
            if (m_SimulateAssetBundleInEditor == -1)
                m_SimulateAssetBundleInEditor = EditorPrefs.GetBool(kSimulateAssetBundles, true) ? 1 : 0;

            return m_SimulateAssetBundleInEditor != 0;
        }
        set
        {
            int newValue = value ? 1 : 0;
            if (newValue != m_SimulateAssetBundleInEditor)
            {
                m_SimulateAssetBundleInEditor = newValue;
                EditorPrefs.SetBool(kSimulateAssetBundles, value);
            }
        }
    }
#endif

    static public LoadedAssetBundle GetBundle(string assetBundleName)
    {

        LoadedAssetBundle bundle = null;
        m_LoadedAssetBundles.TryGetValue(assetBundleName, out bundle);
        return bundle;
    }

    // Get loaded AssetBundle, only return vaild object when all the dependencies are downloaded successfully.
    static public LoadedAssetBundle GetLoadedAssetBundle(string assetBundleName)
    {

        LoadedAssetBundle bundle = null;
        m_LoadedAssetBundles.TryGetValue(assetBundleName, out bundle);
        if (bundle == null)
            return null;

        // No dependencies are recorded, only the bundle itself is required.
        string[] dependencies = null;
        if (!m_Dependencies.TryGetValue(assetBundleName, out dependencies))
            return bundle;

        // Make sure all dependencies are loaded
        foreach (var dependency in dependencies)
        {

            // Wait all the dependent assetBundles being loaded.
            LoadedAssetBundle dependentBundle;
            m_LoadedAssetBundles.TryGetValue(dependency, out dependentBundle);
            if (dependentBundle == null)
                return null;
        }

        return bundle;
    }

    // Load AssetBundleManifest.
    static public AssetBundleLoadManifestOperation Initialize(string manifestAssetBundleName)
    {
        var go = new GameObject("AssetBundleManager", typeof(AssetBundleManager));

#if UNITY_EDITOR
        // If we're in Editor simulation mode, we don't need the manifest assetBundle.
        if (SimulateAssetBundleInEditor)
            return null;
#endif

        LoadAssetBundle(manifestAssetBundleName, true);
        var operation = new AssetBundleLoadManifestOperation(manifestAssetBundleName, "AssetBundleManifest", typeof(AssetBundleManifest));
        m_InProgressOperations.Add(operation);
        return operation;
    }

    static public AssetBundleLoadManifestOperation ReloadManifest(string manifestAssetBundleName)
    {
        UnloadAssetBundleInternal(manifestAssetBundleName);
        LoadAssetBundle(manifestAssetBundleName, true);
        var operation = new AssetBundleLoadManifestOperation(manifestAssetBundleName, "AssetBundleManifest", typeof(AssetBundleManifest));
        m_InProgressOperations.Add(operation);
        return operation;
    }

    // Load AssetBundle and its dependencies.
    static public void LoadAssetBundle(string assetBundleName, bool isLoadingAssetBundleManifest = false)
    {
#if UNITY_EDITOR
        // If we're in Editor simulation mode, we don't have to really load the assetBundle and its dependencies.
        if (SimulateAssetBundleInEditor)
            return;
#endif

        if (!isLoadingAssetBundleManifest)
            assetBundleName = RemapVariantName(assetBundleName);

        // Check if the assetBundle has already been processed.
        //bool isAlreadyProcessed = LoadAssetBundleInternal(assetBundleName, isLoadingAssetBundleManifest);
        LoadAssetBundleInternal(assetBundleName, isLoadingAssetBundleManifest);

        // Load dependencies.
        //if (!isAlreadyProcessed && !isLoadingAssetBundleManifest)
        if (!isLoadingAssetBundleManifest)
            LoadDependencies(assetBundleName);
    }

    // Remaps the asset bundle name to the best fitting asset bundle variant.
    static protected string RemapVariantName(string assetBundleName)
    {
        string[] bundlesWithVariant = m_AssetBundleManifest.GetAllAssetBundlesWithVariant();

        // If the asset bundle doesn't have variant, simply return.
        if (System.Array.IndexOf(bundlesWithVariant, assetBundleName) < 0)
            return assetBundleName;

        string[] split = assetBundleName.Split('.');

        int bestFit = int.MaxValue;
        int bestFitIndex = -1;
        // Loop all the assetBundles with variant to find the best fit variant assetBundle.
        for (int i = 0; i < bundlesWithVariant.Length; i++)
        {
            string[] curSplit = bundlesWithVariant[i].Split('.');
            if (curSplit[0] != split[0])
                continue;

            int found = System.Array.IndexOf(m_Variants, curSplit[1]);
            if (found != -1 && found < bestFit)
            {
                bestFit = found;
                bestFitIndex = i;
            }
        }

        if (bestFitIndex != -1)
            return bundlesWithVariant[bestFitIndex];
        else
            return assetBundleName;
    }

    static protected void LoadAssetBundleInternal(string assetBundleName, bool isLoadingAssetBundleManifest = true)
    {
        LoadedAssetBundle bundle = null;
        m_LoadedAssetBundles.TryGetValue(assetBundleName, out bundle);
        if (bundle != null)
        {
            bundle.m_ReferencedCount++;
            return;
        }

        if (m_LoadingRequest.ContainsKey(assetBundleName))
        {
            return;
        }
        string url = AssetBundleLoader.Instance().GetBundleUrl(assetBundleName);
		SampleDebuger.Log("ab url: " + url);
#if UNITY_WEBGL && !UNITY_EDITOR
		WWW request = new WWW(url + "?" + VersionManager.Instance().GetVersionUrl());
#elif UNITY_WEBGL
		WWW request = new WWW(url);
#else
		AssetBundleCreateRequest request = AssetBundle.LoadFromFileAsync(url);
#endif
		m_LoadingRequest.Add(assetBundleName, request);
    }

    // Where we get all the dependencies and load them all.
    static protected void LoadDependencies(string assetBundleName)
    {
        if (m_AssetBundleManifest == null)
        {
            SampleDebuger.LogError("Please initialize AssetBundleManifest by calling AssetBundleManager.Initialize()");
            return;
        }

        // Get dependecies from the AssetBundleManifest object..
        string[] dependencies = m_AssetBundleManifest.GetAllDependencies(assetBundleName);
        if (dependencies.Length == 0)
            return;

        for (int i = 0; i < dependencies.Length; i++)
            dependencies[i] = RemapVariantName(dependencies[i]);

        // Record and load all dependencies.(因为现在有的assetbundle资源是可选为不释放的)
        if (!m_Dependencies.ContainsKey(assetBundleName))
            m_Dependencies.Add(assetBundleName, dependencies);
        for (int i = 0; i < dependencies.Length; i++)
            LoadAssetBundleInternal(dependencies[i], false);
    }

    // Unload assetbundle and its dependencies.
    static public void UnloadAssetBundle(string assetBundleName, bool unload = false)
    {
#if UNITY_EDITOR
        // If we're in Editor simulation mode, we don't have to load the manifest assetBundle.
        if (SimulateAssetBundleInEditor)
            return;
#endif
        UnloadAssetBundleInternal(assetBundleName, unload);
        UnloadDependencies(assetBundleName, unload);
    }

    static protected void UnloadDependencies(string assetBundleName, bool unload = false)
    {
        string[] dependencies = null;
        if (!m_Dependencies.TryGetValue(assetBundleName, out dependencies))
            return;

        // Loop dependencies.
        foreach (var dependency in dependencies)
        {
            UnloadAssetBundleInternal(dependency, unload);
        }

        m_Dependencies.Remove(assetBundleName);
    }

    static protected void UnloadAssetBundleInternal(string assetBundleName, bool unload = false)
    {
        LoadedAssetBundle bundle = GetBundle(assetBundleName);

        if (bundle != null && --bundle.m_ReferencedCount == 0)
        {
            bundle.m_AssetBundle.Unload(unload);
            m_LoadedAssetBundles.Remove(assetBundleName);
        }
    }

    void Update()
    {
        if (m_LoadingRequest.Count > 0)
        {
            foreach (var keyValue in m_LoadingRequest)
            {
#if UNITY_WEBGL
				WWW request = keyValue.Value;
#else
				AssetBundleCreateRequest request = keyValue.Value;
#endif
                if (request.isDone&& request.assetBundle!=null)
                {
                    if (!m_LoadedAssetBundles.ContainsKey(keyValue.Key))
                        m_LoadedAssetBundles.Add(keyValue.Key, new LoadedAssetBundle(request.assetBundle));
                    else
                    {
                        AssetBundle ab = m_LoadedAssetBundles[keyValue.Key].m_AssetBundle;
                        if (ab == null)
                        {
                            m_LoadedAssetBundles[keyValue.Key] = new LoadedAssetBundle(request.assetBundle);
                        }
                    }
                    _removeList.Add(keyValue.Key);
                }
            }

            foreach (var key in _removeList)
            {
                m_LoadingRequest.Remove(key);
            }

            _removeList.Clear();
        }
        // Update all in progress operations
        for (int i = 0; i < m_InProgressOperations.Count;)
        {
            if (!m_InProgressOperations[i].Update())
            {
                m_InProgressOperations.RemoveAt(i);
            }
            else
                i++;
        }
    }

    // Load asset from the given assetBundle.
    static public AssetBundleLoadAssetOperation LoadAssetAsync(string assetBundleName, string assetName, System.Type type, bool bSingle = true)
    {
        AssetBundleLoadAssetOperation operation = null;
#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            SampleDebuger.Log("LoadAssetAsync " + assetBundleName);
            operation = new AssetBundleLoadAssetOperationSimulation(assetBundleName, assetName, bSingle);
        }
        else
#endif
        {
            LoadAssetBundle(assetBundleName);
            operation = new AssetBundleLoadAssetOperationFull(assetBundleName, assetName, type, bSingle);

            m_InProgressOperations.Add(operation);
        }

        return operation;
    }

    // Load level from the given assetBundle.
    static public AssetBundleLoadBaseOperation LoadLevelAsync(string assetBundleName, string levelName, bool isAdditive)
    {
        AssetBundleLoadBaseOperation operation = null;
#if UNITY_EDITOR
        if (SimulateAssetBundleInEditor)
        {
            string[] levelPaths = AssetDatabase.GetAssetPathsFromAssetBundleAndAssetName(assetBundleName, levelName);
            if (levelPaths.Length == 0)
            {
                ///@TODO: The error needs to differentiate that an asset bundle name doesn't exist
                //        from that there right scene does not exist in the asset bundle...

                SampleDebuger.LogError("There is no scene with name \"" + levelName + "\" in " + assetBundleName);
                return null;
            }

            AssetBundleLoadLevelSimulationOperation temp = new AssetBundleLoadLevelSimulationOperation();
            if (isAdditive)
                temp.m_sceneRequest = EditorApplication.LoadLevelAdditiveAsyncInPlayMode(levelPaths[0]);
            else
                temp.m_sceneRequest = EditorApplication.LoadLevelAsyncInPlayMode(levelPaths[0]);

            operation = temp;

        }
        else
#endif
        {
            LoadAssetBundle(assetBundleName);
            operation = new AssetBundleLoadLevelOperation(assetBundleName, levelName, isAdditive);

            m_InProgressOperations.Add(operation);
        }

        return operation;
    }

} // End of AssetBundleManager.