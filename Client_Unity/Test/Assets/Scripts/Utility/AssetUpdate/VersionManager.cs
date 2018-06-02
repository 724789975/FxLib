using UnityEngine;
using System.Collections;
using System.IO;
using System;

[Serializable]
public class Version
{
    public int m_dwMainVersion = 0;

    public int m_dwSubVersion = 0;

    public int m_dwMiniVersion = 0;

    public Version()
    {
    }

    public Version(string ver)
    {
        proCurVersion = ver;
    }

    public string proCurVersion
    {
        get
        {
            return string.Format("{0}.{1}.{2}", m_dwMainVersion, m_dwSubVersion, m_dwMiniVersion);
        }
        set
        {
            string tmp = value;
            string[] t = tmp.Split(new char[] { '.' }, System.StringSplitOptions.RemoveEmptyEntries);

            if (t.Length > 2)
            {
                m_dwMainVersion = int.Parse(t[0]);
                m_dwSubVersion = int.Parse(t[1]);
                m_dwMiniVersion = int.Parse(t[2]);
            }
        }
    }

    override public string ToString()
    {
        return proCurVersion;
    }

    public int ToNumber()
    {
        return m_dwMainVersion * 10000 + m_dwSubVersion * 100 + m_dwMiniVersion ;
    }

    public bool IsLower(Version other)
    {
		return ToNumber() < other.ToNumber();
    }

	public bool IsEqual(Version other){
		return m_dwMainVersion == other.m_dwMainVersion && 
			m_dwSubVersion == other.m_dwSubVersion && 
			m_dwMiniVersion == other.m_dwMiniVersion;
	
	}
}


public class VersionManager:MonoBehaviour
{
    static VersionManager _instance = null;

    public Version m_verVersion = new Version("0.0.0");

    bool m_bRead = false;

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
            if(!_instance.m_bRead)
            {
                byte[] content = File.ReadAllBytes(Application.streamingAssetsPath + "/version.txt");
                string ver = System.Text.Encoding.UTF8.GetString(content);

                _instance.proCurVersion = ver;
                _instance.m_bRead = true;
            }
#else
#endif
            return _instance;
        }
    }

    public string proCurVersion
    {
        get
        {
            return m_verVersion.proCurVersion;
        }
        set
        {
            m_verVersion.proCurVersion = value;
            SaveVersion(Application.persistentDataPath + "/version.txt");
        }
    }

    public float GetVersionNum()
    {
        return m_verVersion.ToNumber();
    }

    public string GetVersionUrl()
    {
        return proCurVersion.Replace(".", "_");
    }

	public static string GetVersionUrl(string szVersion)
	{
		return szVersion.Replace(".", "_");
	}

    public void SaveVersion(string szPath)
    {
        File.WriteAllBytes(szPath, System.Text.Encoding.UTF8.GetBytes(m_verVersion.ToString()));
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
        cb(m_verVersion.IsLower(srvVersion));
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
