using UnityEngine;
using System.Collections;

public class AssetPoolItem : MonoBehaviour {
    private string _assetName;

    public void SetAssetName(string name)
    {
        _assetName = name;
    }

    public string GetAssetName()
    {
        return _assetName;
    }

    public void Drop()
    {
        this.gameObject.SetActive(false);
        AssetManager. Instance().PutEnityToPool(this);
    }


}
