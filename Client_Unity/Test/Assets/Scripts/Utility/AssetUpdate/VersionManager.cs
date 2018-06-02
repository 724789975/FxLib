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


public class VersionManager : SingletonObject<VersionManager>
{
    public Version m_verVersion = new Version("0.0.0");

	void Awake()
	{
		DontDestroyOnLoad(this);
		CreateInstance(this);
		byte[] content = File.ReadAllBytes(Application.streamingAssetsPath + "/version.txt");
		string ver = System.Text.Encoding.UTF8.GetString(content);

		proCurVersion = ver;
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
}
