using UnityEngine;
using System.Collections;

public static class Globals
{
    public static string streamingPath = Application.streamingAssetsPath;
	public static string persistenPath = Application.persistentDataPath;
#if UNITY_ANDROID
	public static string wwwStreamingPath = Application.streamingAssetsPath;
	public static string wwwPersistenPath = "file:///" + Application.persistentDataPath; 
#elif UNITY_WEBGL
	public static string wwwStreamingPath = Application.streamingAssetsPath;
	public static string wwwPersistenPath = Application.persistentDataPath;
#else
	public static string wwwStreamingPath = "file:///" + Application.streamingAssetsPath;
	public static string wwwPersistenPath = "file:///" + Application.persistentDataPath;
#endif

    public static string NoticeJson;
}
