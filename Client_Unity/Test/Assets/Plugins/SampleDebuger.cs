using UnityEngine;
using Junfine.Debuger;
using System.Runtime.InteropServices;

/// <summary>
/// Log 封装类
/// </summary>
public class SampleDebuger
{
	[DllImport("__Internal")]
	private static extern void LogStr(string szLog);

	[DllImport("__Internal")]
	private static extern void AlertString(string szLog);

#if RELEASE_VER
	static private bool enableLog = true;
	static private bool enableLogAll = true;//启用所有log
#else
	static private bool enableLog = true;
	static private bool enableLogAll = true; //启用所有log
#endif
	static private bool enableLogError = true;
	static private bool enableLogWarning = true;

	static public void Log(object message)
	{
		if (enableLogAll && enableLog)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			LogStr(message.ToString());
#else
			Debuger.Log(message.ToString());
#endif
		}
	}

	static public void Log(object message, Object context)
	{
		if (enableLogAll && enableLog)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			LogStr(message.ToString());
#else
			Debuger.Log(message.ToString(), context);
#endif
		}
	}

	static public void LogColor(object message, string color = "green")
	{
		if (enableLogAll && enableLog)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			LogStr(string.Format("color: {0};{1}", color, message.ToString()));
#else
			Debuger.Log(string.Format("<color={0}>{1}</color>", color, message.ToString()));
#endif
		}
	}

	static public void LogGreen(object message)
	{
		LogColor(message, "green");
	}

	static public void LogRed(object message)
	{
		LogColor(message, "red");
	}

	static public void LogBlue(object message)
	{
		LogColor(message, "blue");
	}

	static public void LogYellow(object message)
	{
		LogColor(message, "yellow");
	}

	static public void LogError(object message)
	{
		if (enableLogAll && enableLog)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			AlertString(message.ToString());
#else
			Debuger.LogError(message.ToString());
#endif
		}
	}

	static public void LogError(object message, Object context)
	{
		if (enableLogAll && enableLogError)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			AlertString(message.ToString());
#else
			Debuger.LogError(message.ToString(), context);
#endif
		}
	}

	static public void LogWarning(object message)
	{
		if (enableLogAll && enableLogError)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			LogStr(message.ToString());
#else
			Debuger.LogWarning(message.ToString());
#endif
		}
	}

	static public void LogWarning(object message, Object context)
	{
		if (enableLogAll && enableLogWarning)
		{
#if UNITY_WEBGL && !UNITY_EDITOR
			LogStr(message.ToString());
#else
			Debuger.LogWarning(message.ToString(), context);
#endif
		}
	}
}
