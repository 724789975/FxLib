using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class H5Helper : MonoBehaviour
{
	[DllImport("__Internal")]
	private static extern string GetUrl();

	[DllImport("__Internal")]
	private static extern void H5LogStr(string szLog);

	public static void LogStr(string szLog)
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		H5LogStr(szLog);
#else
		Debug.Log(szLog);
#endif
	}

	public static string GetH5Url()
	{
		return GetUrl();
	}
}
