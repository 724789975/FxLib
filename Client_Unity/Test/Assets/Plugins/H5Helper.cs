using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class H5Helper : MonoBehaviour
{
	[DllImport("__Internal")]
	private static extern string GetUrl();

	public static string H5GetUrl()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		return GetUrl();
#else
		return "not in webgl!!!!!!";
#endif
	}

	public static IEnumerator SendGet(string _url, System.Action<string> cbf)
	{
		WWW getData = new WWW(_url);
		yield return getData;
		if (getData.error != null)
		{
			SampleDebuger.Log(getData.error);
		}
		else
		{
			cbf(getData.text);
		}
	}

	public static IEnumerator SendPost(string _url, WWWForm _wForm, System.Action<string> cbf)
	{
		WWW postData = new WWW(_url, _wForm);
		yield return postData;
		if (postData.error != null)
		{
			SampleDebuger.Log(postData.error);
		}
		else
		{
			cbf(postData.text);
		}
	}

	public static IEnumerator SendGet(string _url, System.Action<Texture2D> cbf)
	{
		WWW imageData = new WWW(_url);
		yield return imageData;
		if (imageData.error != null)
		{
			SampleDebuger.Log(imageData.error);
		}
		else
		{
			cbf(imageData.texture);
		}
	}

}
