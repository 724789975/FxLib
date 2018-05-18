using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//一个场景只有一个进度条
public class UIProgressBar : UiSingleton<UIProgressBar>
{
	void Awake()
	{
		CreateInstance(this);
	}
	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
		if (m_pAssetBundleLoadOperation != null)
		{
			m_sliderProgressBar.value = m_pAssetBundleLoadOperation.GetProgress();
			m_textLoading.text = (m_sliderProgressBar.value * 100).ToString() + "% loading...";
        }
	}

	public static void SetpAssetBundleLoadOperation(AssetBundleLoadOperation pAssetBundleLoadOperation)
	{
		if (Instance() == null)
		{
			return;
		}

		Instance().m_pAssetBundleLoadOperation = pAssetBundleLoadOperation;
	}

	AssetBundleLoadOperation m_pAssetBundleLoadOperation = null;

	public UnityEngine.UI.Slider m_sliderProgressBar;
	public UnityEngine.UI.Text m_textLoading;
}
