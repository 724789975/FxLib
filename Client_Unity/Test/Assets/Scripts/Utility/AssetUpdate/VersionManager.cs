using UnityEngine;
using System.Collections;
using System.IO;
using System;

public class Version
{
    public int mainVersion = 0;

    public int subVersion = 0;

    public int miniVersion = 0;

    public Version()
    {
    }

    public Version(string ver)
    {

        curVersion = ver;
    }

    public string curVersion
    {

        get
        {

            return string.Format("{0}.{1}.{2}", mainVersion, subVersion, miniVersion);
        }
        set
        {
            string tmp = value;
            string[] t = tmp.Split(new char[] { '.' }, System.StringSplitOptions.RemoveEmptyEntries);

            if (t.Length > 2)
            {
                mainVersion = int.Parse(t[0]);
                subVersion = int.Parse(t[1]);
                miniVersion = int.Parse(t[2]);
            }
        }
    }

    override public string ToString()
    {
        return curVersion;
    }

    public int ToNumber()
    {
        return mainVersion * 10000 + subVersion * 100 + miniVersion ;
    }

    public bool IsLower(Version other)
    {
		return ToNumber() < other.ToNumber();
    }

	public bool IsEqual(Version other){
		return mainVersion == other.mainVersion && 
			subVersion == other.subVersion && 
			miniVersion == other.miniVersion;
	
	}
}


public class VersionManager:MonoBehaviour
{

    static VersionManager _instance = null;

    public Version version = new Version("0.0.0");

    bool bRead = false;

    public static VersionManager Instance
    {
        get
        {
            if (_instance == null)
            {
                GameObject obj = new GameObject();
                _instance = obj.AddComponent<VersionManager>();
                DontDestroyOnLoad(obj);
            }

#if UNITY_EDITOR

            // 读取 version.txt
            if(!_instance.bRead)
            {
                byte[] content = File.ReadAllBytes(Application.streamingAssetsPath + "/version.txt");

                string ver = System.Text.Encoding.UTF8.GetString(content);

                _instance.curVersion = ver;
                _instance.bRead = true;
            }
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
            saveVersion(Application.persistentDataPath + "/version.txt");
#endif

        }
    }

    public float getVersionNum()
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

    IEnumerator _checkServerVersion(Action<bool> cb)
    {
        WWW www = new WWW(GameConstant.g_szPatchUrl + "/version.txt?" + Time.realtimeSinceStartup.ToString()); 

        yield return www;
        if (!string.IsNullOrEmpty(www.error))
        {
            //无法连接资源服务器
            SampleDebuger.LogWarning("url " + www.url + " ,error:" + www.error);
            cb(false);
            yield break;
        }

        if (!www.isDone)
            yield return www;

        Version srvVersion = new Version(www.text.Trim());
        cb(version.IsLower(srvVersion));
    }

    public void CheckServerVersion(Action<bool> cb)
    {
#if RELEASE_VER
        StartCoroutine(_checkServerVersion(cb));
#else
        cb(false);
#endif
    }
}
