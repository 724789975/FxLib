using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LogHelper : MonoBehaviour
{
	public static void LogStr(string szLog)
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		Application.ExternalCall("LogStr", szLog);
#else
		Debug.Log(szLog);
#endif
	}
}
