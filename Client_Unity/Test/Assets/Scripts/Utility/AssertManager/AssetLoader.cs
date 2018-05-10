using UnityEngine;
using System.Collections;

public class AssetLoader : MonoBehaviour {
    private GameObject abNode;
    public string abPath;

	// Use this for initialization
	void Start () {
        if (abNode == null)
        {
            GameObject prefab = AssetManager.Instance().GetEnityPrefab(abPath);
            GameObject uiObj = GameObject.Instantiate(prefab);
            uiObj.transform.SetParent(transform, false);
        }
	}
}
