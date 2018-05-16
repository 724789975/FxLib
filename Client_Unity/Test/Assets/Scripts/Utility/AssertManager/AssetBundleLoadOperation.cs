using UnityEngine;
using System.Collections;
using UnityEngine.SceneManagement;
#if UNITY_EDITOR
using UnityEditor;
#endif
public abstract class AssetBundleLoadOperation : IEnumerator
{
	public object Current
	{
		get
		{
			return null;
		}
	}
	public bool MoveNext()
	{
		return !IsDone();
	}
	
	public void Reset()
	{
	}
	
	abstract public bool Update ();
	
	abstract public bool IsDone ();

	abstract public float GetProgress();
}

public abstract class AssetBundleLoadBaseOperation : AssetBundleLoadOperation
{
    public AsyncOperation m_sceneRequest;
}

public class AssetBundleLoadLevelSimulationOperation : AssetBundleLoadBaseOperation
{
	public AssetBundleLoadLevelSimulationOperation ()
	{
	}
	
	public override bool Update ()
	{
		return false;
	}
	
	public override bool IsDone ()
	{
		return true;
	}

	public override float GetProgress()
	{
		return 0.0f;
	}
}

public class AssetBundleLoadLevelOperation : AssetBundleLoadBaseOperation
{
	protected string 			m_AssetBundleName;
	protected string 			m_LevelName;
	protected bool 				m_IsAdditive;
	protected string 			m_DownloadingError;

	public AssetBundleLoadLevelOperation (string assetbundleName, string levelName, bool isAdditive)
	{
		m_AssetBundleName = assetbundleName;
		m_LevelName = levelName;
		m_IsAdditive = isAdditive;
	}

	public override bool Update ()
	{
        if (m_sceneRequest != null)
			return false;
		
		LoadedAssetBundle bundle = AssetBundleManager.GetLoadedAssetBundle (m_AssetBundleName);
		if (bundle != null)
		{
			if (m_IsAdditive)
                m_sceneRequest = SceneManager.LoadSceneAsync(m_LevelName, LoadSceneMode.Additive);
			else
                m_sceneRequest = SceneManager.LoadSceneAsync(m_LevelName);
			return false;
		}
		else
			return true;
	}
	
	public override bool IsDone ()
	{
		// Return if meeting downloading error.
		// m_DownloadingError might come from the dependency downloading.
        if (m_sceneRequest == null && m_DownloadingError != null)
		{
			Debug.LogError(m_DownloadingError);
			return true;
		}

        return m_sceneRequest != null && m_sceneRequest.isDone;
	}

	public override float GetProgress()
	{
		return m_sceneRequest.progress;
	}
}

public abstract class AssetBundleLoadAssetOperation : AssetBundleLoadOperation
{
	public abstract T GetAsset<T>() where T : UnityEngine.Object;
    public abstract T[] GetAllAsset<T>() where T : UnityEngine.Object;
}

public class AssetBundleLoadAssetOperationSimulation : AssetBundleLoadAssetOperation
{
	Object							m_SimulatedObject;
    Object[]                        m_allObject;

    protected float startTime;
    protected float deltaTime = 0f;

    public AssetBundleLoadAssetOperationSimulation(string assetBundleName, string assetName, bool single)
	{
#if UNITY_EDITOR
        startTime = Time.realtimeSinceStartup;
        if (single)
        {
            m_SimulatedObject =  AssetDatabase.LoadMainAssetAtPath(assetBundleName);
        }
        else
        {
            m_allObject = AssetDatabase.LoadAllAssetsAtPath(assetBundleName);
        }
#endif
	}
	
	public override T GetAsset<T>()
	{
		return m_SimulatedObject as T;
	}
    public override T[] GetAllAsset<T>()
    {
        return m_allObject as T[];
    }

	public override bool Update ()
	{
		return false;
	}
	
	public override bool IsDone ()
	{
        if ((Time.realtimeSinceStartup - startTime) < deltaTime)
            return false;
        else
		    return true;
	}

	public override float GetProgress()
	{
		return 0.0f;
	}
}

public class AssetBundleLoadAssetOperationFull : AssetBundleLoadAssetOperation
{
	protected string 				m_AssetBundleName;
	protected string 				m_AssetName;
	protected System.Type 			m_Type;
	protected AssetBundleRequest	m_Request = null;
    protected AssetBundleRequest    m_AllRequest = null;
    protected bool                  m_IsSingle = true;

	public AssetBundleLoadAssetOperationFull (string bundleName, string assetName, System.Type type, bool bSingle)
	{
		m_AssetBundleName = bundleName;
		m_AssetName = assetName;
		m_Type = type;
        m_IsSingle = bSingle;
     }
	
	public override T GetAsset<T>()
	{
		if (m_Request != null && m_Request.isDone)
			return m_Request.asset as T;
		else
			return null;
	}

    public override T[] GetAllAsset<T>()
    {
        if (m_AllRequest != null)
            return m_AllRequest.allAssets as T[];
        else
            return null;
    }

	// Returns true if more Update calls are required.
	public override bool Update ()
	{
		if (m_Request != null)
			return false;

		LoadedAssetBundle bundle = AssetBundleManager.GetLoadedAssetBundle (m_AssetBundleName);
		if (bundle != null)
		{
            if(m_IsSingle)
                m_Request = bundle.m_AssetBundle.LoadAssetAsync(m_AssetName, m_Type);
            else
                m_AllRequest = bundle.m_AssetBundle.LoadAllAssetsAsync(m_Type);
			return false;
		}
		else
		{
			return true;
		}
	}
	
	public override bool IsDone ()
	{
        if (m_IsSingle)
            return m_Request != null && m_Request.isDone;
        else
            return m_AllRequest != null && m_AllRequest.isDone;
	}

	public override float GetProgress()
	{
		return m_Request.progress + m_AllRequest.progress;
	}
}

public class AssetBundleLoadManifestOperation : AssetBundleLoadAssetOperationFull
{
	public AssetBundleLoadManifestOperation (string bundleName, string assetName, System.Type type)
		: base(bundleName, assetName, type, true)
	{
	}

	public override bool Update ()
	{
		base.Update();
		
		if (m_Request != null && m_Request.isDone)
		{
			AssetBundleManager.AssetBundleManifestObject = GetAsset<AssetBundleManifest>();
            SampleDebuger.Log("AssetBundleManifest was been loaded!");
			return false;
		}
		else
			return true;
	}
}

