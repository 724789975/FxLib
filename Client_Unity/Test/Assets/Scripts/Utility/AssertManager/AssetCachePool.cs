using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public delegate void CallBackWithPercent(int done, int total);

public class ABPrefabInfo
{
    GameObject _prefab;
    event Action<GameObject> _loadListener;

    public ABPrefabInfo(Action<GameObject> cb)
    {
        _loadListener += cb;
    }

    public ABPrefabInfo(GameObject obj)
    {
        _prefab = obj; 
    }

    public void AddListener(Action<GameObject> fn){
        _loadListener += fn;
    }

    public void LoadedPrefab(GameObject obj)
    {
        _prefab = obj;
        _loadListener(obj);
    }

    public GameObject GetPrefab()
    {
        return _prefab;
    }
}


public class AssetCachePool : MonoBehaviour
{
    private Dictionary<string, ABPrefabInfo> _cachePrefabs = new Dictionary<string, ABPrefabInfo>();

    public void LoadAsset(string path, Action<GameObject> fn)
    {
        string bundleName = path.ToLower();
        string assetName = path + ".prefab";
        ABPrefabInfo abInfo = null;
        _cachePrefabs.TryGetValue(bundleName, out abInfo);
        if (abInfo != null)
        {
            GameObject prefab =  abInfo.GetPrefab();
            if (prefab)
            {
                //Prefab已被加载
                fn(prefab);
            }
            else
            {
                //Prefab正在加载
                abInfo.AddListener(fn);
            }
        }
        else
        {
            //第一加载Prefab
            abInfo = new ABPrefabInfo(fn);
            _cachePrefabs.Add(bundleName, abInfo);
            AssetBundleLoader.Instance().LoadAsset(bundleName, assetName, (assetObj) =>
            {
                abInfo.LoadedPrefab(assetObj as GameObject);
            });
        }

    }

    public void LoadAssetList(PreloadFileModel model, CallBackWithPercent cb)
    {
        StartCoroutine(OnLoadAssetList(model, cb));
    }

    private IEnumerator OnLoadAssetList(PreloadFileModel model, CallBackWithPercent cb)
    {
        List<string> list = model.fileList;

        for (int i = 0; i < list.Count; i++)
        {
            string path = list[i];
            string bundleName = path.ToLower();
            string assetName = path + ".prefab";

            AssetBundleLoadAssetOperation request = AssetBundleManager.LoadAssetAsync(bundleName, assetName, typeof(UnityEngine.Object));
            if (request != null)
            {
                yield return StartCoroutine(request);
                GameObject obj = request.GetAsset<UnityEngine.GameObject>();

                _cachePrefabs.Add(bundleName, new ABPrefabInfo(obj));
            }
            else
            {
                SampleDebuger.LogError("bundle ++" + bundleName + "++ can't loading");
            }
            cb(i + 1, list.Count);
        }
    }

    //获得Prefab
    public GameObject GetPrefab(string path)
    {
        ABPrefabInfo obj = null;
        _cachePrefabs.TryGetValue(path.ToLower(), out obj);
        return obj.GetPrefab();
    }

    //加载Prefab通过回调返回
    public void GetPrefabAsync(string path, Action<GameObject> fn)
    {
        GameObject obj = GetPrefab(path.ToLower());
        if (obj == null)
        {
            LoadAsset(path, (assetObj) => {
                fn(assetObj);
            });
        }
        else
        {
            fn(obj);
        }
    }


    public void CleanUp()
    {
        foreach (string key in _cachePrefabs.Keys)
        {
            AssetBundleManager.UnloadAssetBundle(key, true);
        }
        _cachePrefabs.Clear();
    }

}
