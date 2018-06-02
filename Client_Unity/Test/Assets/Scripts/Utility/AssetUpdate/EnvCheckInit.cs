using UnityEngine;
using System.Collections;
using System.IO;
using UnityEngine.UI;
using System;

/// <summary>
/// 
/// </summary>
public class EnvCheckInit : MonoBehaviour
{
    public AssetUpdater _assetUpdater;

	//public Text statusText;

	//public Text descText;

	public Slider progressSlider;

	//public Image bottomImage;

	//public Text versionText;
	//public Text tip;

	//public GameObject enterButtonObj;
	//public GameObject loadingObj;

	public string[] tipArr;
    public int tipIndex;

    string currentVersion;

    void Awake()
    {

        //Once("event_lang_initialized", _start);
        //Once("LuaMangager_Started", GameStart);
        //On("Notice_Loaded", _noticeLoaded);
        //Once("MainPlayer_DataLoaded", _playerDataLoaded);
        //Once("ChannelManager_initialized", LocalVerCheck);
        
        //_assetUpdater = GetComponent<AssetUpdater>();
        //progressSlider.gameObject.SetActive(false);
        //enterButtonObj.SetActive(false);
        //loadingObj.SetActive(true);
        //ClientConfig.Load();
        //LangSetting.Initialize();
        
    }

    /// <summary>
    /// 本地版本检查
    /// </summary>
    public void LocalVerCheck(params object[] args)
    {
        //progressSlider.gameObject.SetActive(true);
		StartCoroutine(checkVersion());
    }

    /// <summary>
    /// 版本检查
    /// </summary>
    /// <returns></returns>
    IEnumerator checkVersion()
    {
        string path = Globals.wwwPersistenPath + "/version.txt";
        WWW www = new WWW(path);
        yield return www;
        SampleDebuger.Log("version  = " + www.text);

        if (string.IsNullOrEmpty(www.text) || (www.error != null))
        { //没读取到，是第一次安装，拷贝资源
            SampleDebuger.Log("First Time Launch!");
            //读取应用程序版本号
            www = new WWW(Globals.wwwStreamingPath + "/version.txt");
            yield return www;
            currentVersion = www.text.Trim();
            //versionText.text = currentVersion;
            beginCopy();
            www.Dispose();
        }
        else
        { //已安装过
            SampleDebuger.Log(" installed");
            string oldVersion = www.text.Trim();       //读取当前旧版本号

            //读取应用程序版本号
            www = new WWW(Globals.wwwStreamingPath + "/version.txt");
            yield return www;
            currentVersion = www.text.Trim();
            //versionText.text = currentVersion;
            //版本号小于安装程序中包含的版本号，删除旧资源再拷贝当前资源

            Version old_v = new Version(oldVersion);
            Version app_v = new Version(currentVersion);
            if (old_v.IsLower(app_v))
            {
                string abPath = Application.persistentDataPath + "/AssetBundles";
                FileUtil.RemoveFolder(abPath);
                beginCopy();
            }
            else
            {
                VersionManager.Instance.curVersion = oldVersion;
                _assetUpdater.CheckVersionWithServer();
            }
        }
    }

    /// <summary>
    /// 开始下载
    /// </summary>
    public void StartDownload()
    {
		SampleDebuger.Log("start down load");
        _assetUpdater.StartDownload();
    }

    /// <summary>
    /// 更新下载进度
    /// </summary>
    /// <param name="done"></param>
    /// <param name="total"></param>
    public void downloadProcess(long done, long total)
    {
		SampleDebuger.Log("++ done: " + done + " total: " + total);
		progressSlider.value = done > 0 ? (float)done / (float)total : 0;
		//descText.text = LangSetting.GetWord("UI_DownLoadingRes_desc");
		//statusText.text = Mathf.Floor(done / 1024).ToString() + "KB /" + Mathf.Floor(total / 1024).ToString() + "KB";
	}

    /// <summary>
    /// 更新拷贝进度
    /// </summary>
    /// <param name="done"></param>
    /// <param name="total"></param>
    /// <param name="content"></param>
    public void setProcess(int done, int total, string content = null)
    {
		float percent = done > 0.0f ? (float)done / (float)total : 0.0f;
		progressSlider.value = percent;
		//statusText.text = ((int)(percent * 100)).ToString() + "%";
		//if (!string.IsNullOrEmpty(content))
		//    descText.text = content; //"游戏初始化中，此过程不消耗流量！ ";
	}

    ///<summary>
    ///开始拷贝, 完成更新应用程序的同步过程
    ///即解压安装过程
    /// </summary>
    private void beginCopy()
    {
        StartCoroutine(_beginCopy(Globals.wwwStreamingPath));
    }

    /// <summary>
    /// 将文件拷贝到路径中 
    /// </summary>
    /// <param name="streamPath"></param>
    /// <returns></returns>
    IEnumerator _beginCopy(string path)
    {
        yield return new WaitForSeconds(2.0f);
        WWW www = new WWW(path + "/streamPath.txt");
        yield return www;

        string[] content = www.text.Split(new string[] { "\n" }, System.StringSplitOptions.RemoveEmptyEntries);
        www.Dispose();
        int total = content.Length;
        int curIndex = 0;
        foreach (string item in content)
        {
            string it = item.Trim(); //window下会有\r，需要删除
            int fileFlag = int.Parse(it.Split('|')[1]);
            it = it.Split('|')[0];
            SampleDebuger.Log(path);
            it = it.Trim();

            if (fileFlag == 1)
            {
                www = new WWW(path + it);
                yield return www;

                File.WriteAllBytes(Application.persistentDataPath + it, www.bytes);
                //更新进度
                setProcess(curIndex, total);
                www.Dispose();

            }
            else if (fileFlag == 0)
            {
                SampleDebuger.Log("Create dir " + Application.persistentDataPath + it);
                Directory.CreateDirectory(Application.persistentDataPath + it);
            }
            else
            {
                SampleDebuger.LogError("既不是文件夹也不是文件 路径为" + Application.persistentDataPath + it);
            }
            ++curIndex;
        }

        SampleDebuger.Log(" writeversion");

        // 同步版本
        VersionManager.Instance.curVersion = currentVersion;
        _assetUpdater.CheckVersionWithServer();
    }

    public void GameInit()
    {
        //InvokeRepeating("_rollTips", 0, 2);
        StartCoroutine(_gameInit());
    }

    IEnumerator _gameInit()
    {
        //LuaManager.Initialize();
        yield return null;
		AssetBundleLoader.Instance().LoadLevelAsset(GameConstant.g_szChoseServerScene);
	}

    void GameStart(params object[] arg)
    {
        StartCoroutine(_gameStart());
    }

    IEnumerator _gameStart()
    {
		//LuaManager.Instance.OpenLibs();
		//setProcess(3, 20);
		//yield return 1;
		//LuaManager.Instance.Setup();
		//setProcess(4, 20);
		//yield return 1;
		//LuaManager.Instance.Setup2();
		//setProcess(11, 20);
		//yield return 1;
		//LuaManager.Instance.Setup3();
		//setProcess(12, 20);
		//yield return 1;
		//LuaManager.Instance.Setup4();
		//setProcess(13, 20);
		//yield return 1;
		//LuaManager.Instance.StartMain();
		//setProcess(18, 20);
		//yield return 1;
		//LuaManager.Instance.StartLooper();
		//setProcess(19, 20);
		//yield return 1;
		//string bankStr = LuaManager.Instance.CallFunction("GetWwiseBankNames")[0].ToString();
		//Debug.Log("bankStr " + bankStr);
		//AudioCtrl.Init(bankStr);
		//setProcess(20, 20);
		yield return 1;
		AssetManager.Instance().PreloadAsset((done, total) =>
        {
            setProcess(done, total);
            if (done >= total)
            {
                //UGuiManager.Initialize();
                //EventDispatcher.Instance.Emit("GetNotcie");
            }
        });
    }

    //void _rollTips()
    //{
    //    if (tipIndex >= tipArr.Length - 1)
    //    {
    //        tipIndex = 0;
    //    }
    //    descText.text = LangSetting.GetWord(tipArr[tipIndex]);
    //    tipIndex++;
    //}

    void _noticeLoaded(params object[] arg)
    {
        //loadingObj.SetActive(false);
        LoginGame();
    }

    public void LoginGame()
    {
        //ChannelManager.Instance.RequirePermission(8, (ret) =>
        //{
        //    if (ret)
        //    {
        //        EventDispatcher.Instance.Emit("ResetLoginManager");
        //    }
        //    else
        //    {
        //        enterButtonObj.SetActive(!ret);
        //    }
        //});

    }

    void _playerDataLoaded(params object[] args)
    {
        AssetBundleLoader.Instance().LoadLevelAsset("home", () =>
        {
            AssetBundleManager.UnloadAssetBundle("assets/abres/scene/updater", true);
        });
    }

    void OnDestroy()
    {
        //Off("Notice_Loaded", _noticeLoaded);
    }
}
