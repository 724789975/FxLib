using UnityEngine;
using System.Collections;

public static class Globals
{
    public static string streamingPath;
    public static string persistenPath;

    public static string wwwStreamingPath;
    public static string wwwPersistenPath;

    public static string NoticeJson;

    public static void Init()
    {
        Globals.streamingPath = Application.streamingAssetsPath;
        Globals.persistenPath = Application.persistentDataPath;
       
#if UNITY_ANDROID
        Globals.wwwStreamingPath = Application.streamingAssetsPath;
		Globals.wwwPersistenPath = "file:///" + Application.persistentDataPath; 
#elif UNITY_WEBGL
        Globals.wwwStreamingPath = Application.streamingAssetsPath;
        Globals.wwwPersistenPath = Application.persistentDataPath;
#else
        Globals.wwwStreamingPath = "file:///" + Application.streamingAssetsPath;
        Globals.wwwPersistenPath = "file:///" + Application.persistentDataPath;
#endif
	}
}
