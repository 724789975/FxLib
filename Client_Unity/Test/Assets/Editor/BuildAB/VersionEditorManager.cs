using UnityEngine;
using System.Collections;
using System.IO;
using System;

public class VersionEditorManager
{

	static VersionEditorManager _instance = new VersionEditorManager();

    public Version version = new Version("0.0.0");

	public static VersionEditorManager Instance
    {

        get
        {
            if (_instance == null)
            {
				_instance = new VersionEditorManager ();
            }

#if UNITY_EDITOR

            // 读取 version.txt
            byte[] content = File.ReadAllBytes(Application.streamingAssetsPath + "/version.txt");

            string ver = System.Text.Encoding.UTF8.GetString(content);

            _instance.curVersion = ver;
#else


#endif

            return _instance;
        }
    }

    public string curVersion
    {

        get
        {

            return version.curVersion;
        }
        set
        {

            version.curVersion = value;

#if UNITY_EDITOR
            
            saveVersion(Application.streamingAssetsPath + "/version.txt");
#else
            saveVersion(Application.persistentDataPath +  "/version.txt");
#endif

        }
    }

    public int getVersionNum()
    {
        return version.ToNumber();
    }

    public string getVersionUrl()
    {
        return curVersion.Replace(".", "_");
    }

    public void saveVersion(string path)
    {
        File.WriteAllBytes(path, System.Text.Encoding.UTF8.GetBytes(version.ToString()));
		//SampleDebuger.Log(" Update version info");
    }
}
