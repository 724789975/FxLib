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
		return GetUrl();
	}
	public static void H5AlertString(string szLog)
	{
		AlertString(szLog);
	}
}
