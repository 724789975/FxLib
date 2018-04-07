using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class H5Helper : MonoBehaviour
{
	[DllImport("__Internal")]
	private static extern string GetUrl();

	[DllImport("__Internal")]
	private static extern void LogStr(string szLog);

	[DllImport("__Internal")]
	private static extern void AlertString(string szLog);

	public static void H5LogStr(string szLog)
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		LogStr(szLog);
#else
		Debug.Log(szLog);
#endif
	}

	public static string H5GetUrl()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		return GetUrl();
#else
		return "not in webgl!!!!!!";
#endif
	}
	public static void H5AlertString(string szLog)
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		AlertString(szLog);
#else
		Debug.LogError(szLog);
#endif
	}

    public delegate void CallBackFun(string szData);

    public static IEnumerator SendGet(string _url, CallBackFun cbf)
    {
        WWW getData = new WWW(_url);
        yield return getData;
        if (getData.error != null)
        {
            H5LogStr(getData.error);
        }
        else
        {
            cbf(getData.text);
        }
    }

    public static IEnumerator SendPost(string _url, WWWForm _wForm, CallBackFun cbf)
    {
        WWW postData = new WWW(_url, _wForm);
        yield return postData;
        if (postData.error != null)
        {
            H5LogStr(postData.error);
        }
        else
        {
            cbf(postData.text);
        }
    }

}
