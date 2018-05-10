using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;

/// <summary>
/// 预加载的assetbundle的集合
/// </summary>
public class PreloadFileModel
{
    public List<string> fileList = new List<string>();

    public void Add(string file)
    {
        if (!fileList.Contains(file))
        {
            fileList.Add(file);
        }
    }
}

/// <summary>
/// 资源管理类
/// </summary>
public class AssetManager : SingletonObject<AssetManager>
{
    private AssetCachePool _longTermAssetPool;//常驻资源

    private AssetCachePool _shortAssetPool; //暂存资源

    private const string _cellEnityPath = "Assets/ABRes/Entity/"; //场景事件资源路径

    private const string _uipath = "Assets/ABRes/UI/";        //界面资源路径

    private Dictionary<string, Queue<AssetPoolItem>> _entityPoolMap = new Dictionary<string,Queue<AssetPoolItem>>(); //

    void Awake()
    {
		DontDestroyOnLoad(this);
		CreateInstance(this);
		_longTermAssetPool = gameObject.AddComponent<AssetCachePool>();
		_shortAssetPool = gameObject.AddComponent<AssetCachePool>();
    }

    #region 常驻资源
    /// <summary>
    /// 预加载游戏
    /// </summary>
    /// <param name="cb">回调:返回读取进度</param>
    public void PreloadAsset(CallBackWithPercent cb)
    {
        PreloadFileModel preloadModel = FileUtil.ReadJsonFromFile<PreloadFileModel>("Config/PreloadAB.json");
        _longTermAssetPool.LoadAssetList(preloadModel, cb);
    }

    /// <summary>
    /// 获取已加载的界面Prefab
    /// </summary>
    /// <param name="assetName">资源名</param>
    /// <returns>Prefab</returns>
    public GameObject GetUIPrefab(string assetName)
    {
        return _longTermAssetPool.GetPrefab(string.Format("{0}{1}", _uipath, assetName));
    }

    /// <summary>
    /// 获取已加载的游戏实体Prefab
    /// </summary>
    /// <param name="assetName">资源名</param>
    /// <returns>Prefab</returns>
    public GameObject GetEnityPrefab(string assetName)
    {
        return _longTermAssetPool.GetPrefab(string.Format("{0}{1}", _cellEnityPath, assetName));
    }
    #endregion

    #region 临时资源
    public void GetUIPrefabAsync(string assetName, Action<GameObject> cb)
    {
        _longTermAssetPool.LoadAsset(string.Format("{0}{1}", _uipath, assetName), cb);
    }

    public void GetPrefabAsync(string path, Action<GameObject> cb)
    {
        _longTermAssetPool.LoadAsset(path, cb);
    }

    public void GetEntityPrefabAsync(string assetName, Action<GameObject> cb){
        _shortAssetPool.LoadAsset(string.Format("{0}{1}", _cellEnityPath, assetName), cb);
    }

    public void CleanShortAssets()
    {
        _shortAssetPool.CleanUp();
    }

    #endregion

    #region 对象池
    /// <summary>
    /// 从对象池取出一个实体
    /// </summary>
    /// <param name="assetName"></param>
    /// <returns></returns>
    public GameObject GetEnityFromPool(string assetName)
    {
        Queue<AssetPoolItem> pool = null;

        _entityPoolMap.TryGetValue(assetName, out pool);
        GameObject obj = null;
        if (pool != null && pool.Count > 0)
        {
            AssetPoolItem item = pool.Dequeue();
            obj = item.gameObject;
            obj.SetActive(true);
        }
        else
        {
            GameObject prefab = GetEnityPrefab(assetName);
            obj = GameObject.Instantiate<GameObject>(prefab);
            if (obj != null)
            {
                AssetPoolItem item = obj.GetComponent<AssetPoolItem>();
                if (item != null)
                {
                    item.SetAssetName(assetName);
                }
                else
                {
                    SampleDebuger.LogError("asset " + assetName + " is not a pool item");
                }
            }
        }
        return obj;
    }


    public void GetEnityFromPoolAsync(string assetName, Action<GameObject> cb)
    {
        Queue<AssetPoolItem> pool = null;

        _entityPoolMap.TryGetValue(assetName, out pool);
        GameObject obj = null;
        if (pool != null && pool.Count > 0)
        {
            AssetPoolItem item = pool.Dequeue();
            obj = item.gameObject;
            obj.SetActive(true);
            cb(obj);
        }
        else
        {
            GetEntityPrefabAsync(assetName, (prefab) => {
                obj = GameObject.Instantiate<GameObject>(prefab);
                if (obj != null)
                {
                    AssetPoolItem item = obj.GetComponent<AssetPoolItem>();
                    if (item != null)
                    {
                        item.SetAssetName(assetName);
                    }
                    else
                    {
                        SampleDebuger.LogError("asset " + assetName + " is not a pool item");
                    }
                    cb(obj);
                }
            
            });

        }
    }

    /// <summary>
    /// 将资源放回对象池
    /// </summary>
    /// <param name="item"></param>
    public void PutEnityToPool(AssetPoolItem item)
    {
        string assetName = item.GetAssetName();
        Queue<AssetPoolItem> pool = null;
        _entityPoolMap.TryGetValue(assetName, out pool);
        if (pool == null)
        {
            pool = new Queue<AssetPoolItem>();
            _entityPoolMap[assetName] = pool;
        }
        pool.Enqueue(item);
    }

    /// <summary>
    /// 清空对象池
    /// </summary>
    public void CleanEnityPool()
    {
        _entityPoolMap.Clear();
    }

    public Dictionary<string, Queue<AssetPoolItem>> GetEnityPool()
    {
        return _entityPoolMap;
    }

    #endregion


    /// <summary>
    /// 异步加载精灵
    /// </summary>
    /// <param name="path">贴图路径</param>
    /// <param name="spName">精灵名称</param>
    /// <param name="fn">返回精灵回调</param>
    public void LoadSpriteAsync(string path, string spName, Action<Sprite> fn)
    {
        string bundleName = path.ToLower() + ".img";
        string assetName = path + ".png";
        AssetBundleLoader.Instance().LoadAllAsset(bundleName, assetName,
        (arr) =>
        {
            //返回一个精灵列表
            Sprite sprite = null;
            if (arr != null)
            {
                for (int i = 0; i < arr.Length; i++)
                {
                    Sprite obj = arr[i] as Sprite;
                    //遍历寻找目标精灵
                    if (obj != null && obj.name == spName)
                    {
                        sprite = obj;
                        break;
                    }
                }
            }
            //返回精灵
            fn(sprite);
        });
    }

    void OnDestroy()
    {
    }
}
