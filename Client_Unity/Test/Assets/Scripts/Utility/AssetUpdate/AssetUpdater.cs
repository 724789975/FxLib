using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using Junfine.Debuger;
using System;
using System.Threading;
using System.ComponentModel;

/// <summary>
/// ======================== 资源更新 ========================
/// </summary>
public class AssetUpdater : MonoBehaviour
{
    //const string VERSION_URL = "version.txt";
    const string COMMON_URL  = "Common.zip";
	const string CONTENT_URL = "mark.txt";
    private bool _isDownloading = false;
    private string _contentUrl = "";
    public Version srvVersion;
    public EnvCheckInit _envChecker;

    public void Awake()
    {
        //_envChecker = GetComponent<EnvCheckInit>();
    }

    public void Update()
    {
        if (_isDownloading)
        {
            lock (AssetDownloader.WebLock)
            {
                if (AssetDownloader.Intance().bFinished)
                {
                    _isDownloading = false;
                    _envChecker.LocalVerCheck();
                }
                else
                {
                    _envChecker.downloadProcess(AssetDownloader.Intance().Loaded, AssetDownloader.Intance().Total);
                }
            }
        }
    }

    /// <summary>
    /// 获取服务器版本
    /// </summary>
    /// <returns></returns>
    private WWW getVersionWWW()
    {
		return new WWW(GameInstance.Instance().proServerUrl
			+ GameInstance.Instance().proServerVersion + "?" + Time.realtimeSinceStartup.ToString());
	}

    /// <summary>
    /// 获取更新目录
    /// </summary>
    /// <returns></returns>
    private WWW getContentWWW()
	{
		string url = _contentUrl + "/";
		url += VersionManager.Instance.getVersionUrl() + "/";
		url += CONTENT_URL;
		SampleDebuger.Log ("getContentURL:  " + url);
		return new WWW(url + "?" + Time.realtimeSinceStartup.ToString());
	}

    /// <summary>
    /// 获取共用资源包
    /// </summary>
    /// <returns></returns>
    private string getCommonURL()
    {
        string url = _contentUrl + "/";
        url += VersionManager.Instance.getVersionUrl() + "/";
        url += COMMON_URL;
        return url;
    }

    /// <summary>
    /// 获取本平台资源包
    /// </summary>
    /// <returns></returns>
    private string getCustomURL()
    {
        string url = _contentUrl + "/";
        url += VersionManager.Instance.getVersionUrl() + "/";
        url += SysUtil.GetPlatformName() + ".zip"; 
        return url;
    }

    /// <summary>
    /// 检查版本
    /// </summary>
    public void CheckVersionWithServer()
    {
		StartCoroutine(getServerVersion());
    }

    IEnumerator getServerVersion()
    {
		string szVersion = GameInstance.Instance().proServerUrl
			+ GameInstance.Instance().proServerVersion + "?" + Time.realtimeSinceStartup.ToString();
		WWW www = new WWW(szVersion);

        yield return www;
        if (!string.IsNullOrEmpty(www.error))
        {
			//无法连接资源服务器
			SampleDebuger.LogWarning ("url " + www.url + " ,error:" + www.error);
            _envChecker.GameInit();
            yield break;
        }

        if (!www.isDone)
            yield return www;

        string versionStr = www.text.Trim();
        srvVersion = new Version(versionStr);
        SampleDebuger.Log(" server version = " + srvVersion.ToString());
        if (VersionManager.Instance.version.IsLower(srvVersion))
        {
            StartCoroutine(checkVersionContent());
        }
        else
        {
            //进入游戏
            _envChecker.GameInit();
        }  
    }

	IEnumerator checkVersionContent()
	{
		string szVersionContent = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerVersionContent, VersionManager.Instance.getVersionUrl()) + "?" + Time.realtimeSinceStartup.ToString();
		WWW www = new WWW(szVersionContent);
		SampleDebuger.Log ("+++++++++++ checkVersionContent +++++++++++ ");
		yield return www;
		if (!string.IsNullOrEmpty(www.error))
		{
			SampleDebuger.Log(szVersionContent);
			AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szConfirmPanel), GameObjectConstant.g_szConfirmPanel, delegate (UnityEngine.Object ob)
				{
					GameObject go_RoleList = Instantiate((GameObject)ob, MainCanvas.Instance().transform);
					go_RoleList.GetComponent<ConfirmPanel>().Init("检测到大版本更新，请前往官网或者应用商店下载最新版本",
						delegate(object ob1)
						{
							//todo 下载
							SampleDebuger.LogGreen("begin down load");
						}, null, delegate (object ob1)
						{
							//todo 取消下载
							SampleDebuger.LogGreen("cancel down load");
						},null
					);
				}
			);
			yield break;
		}
        //下载更新包
        if (Application.internetReachability == NetworkReachability.ReachableViaLocalAreaNetwork)
        {
            _envChecker.StartDownload();
        }
        else
		{
			AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szConfirmPanel), GameObjectConstant.g_szConfirmPanel, delegate (UnityEngine.Object ob)
				{
					GameObject go_RoleList = Instantiate((GameObject)ob, MainCanvas.Instance().transform);
					go_RoleList.GetComponent<ConfirmPanel>().Init("检测到游戏更新,请点击更新!",
						delegate (object ob1)
						{
							_envChecker.StartDownload();
						}, null, delegate (object ob1)
						{
							//todo 取消下载
							SampleDebuger.LogGreen("cancel down load");
						}, null
					);
				}
			);
        }
	}

    public void StartDownload()
    {
        _isDownloading = true;
		string szVersionContent = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerVersionContent, VersionManager.Instance.getVersionUrl())
			+ "?" + Time.realtimeSinceStartup.ToString();
		AssetDownloader.Intance().AddURL(szVersionContent);
		string szVersion = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerNextVersionPath, VersionManager.Instance.getVersionUrl())
			+ "?" + Time.realtimeSinceStartup.ToString();
		AssetDownloader.Intance().AddURL(szVersion);
		//AssetDownloader.Intance().AddURL(getCommonURL());
		//AssetDownloader.Intance().AddURL(getCustomURL());
		AssetDownloader.Intance().Start();
    }
}

#region 大文件下载器
public class AssetDownloader
{
    private static AssetDownloader _intance = null;
    public static object WebLock = new object();//线程锁
    private WebClient _webClient;
    private Queue<string> _fileQueue = new Queue<string>();
    private string _upzipPath;
    private string _tempPath;


    //status
    public bool bFinished = false;
    public long Loaded = 0;
    public long Total = 0;

    public int _curFileCount = 0 ;


    public static AssetDownloader Intance()
    {
        if (_intance == null)
        {
            _intance = new AssetDownloader();
        }
        return _intance;
    }

    public void AddURL(string url)
    {
        _fileQueue.Enqueue(url);
    }

    public void init()
    {
		_upzipPath = Application.persistentDataPath;
        _tempPath = Application.temporaryCachePath;

        _webClient = new WebClient();

        _curFileCount = 0;

        _webClient.DownloadFileCompleted += new AsyncCompletedEventHandler(onDownloadCompelete);

        _webClient.DownloadProgressChanged += new DownloadProgressChangedEventHandler(onDownloadProcess);

    }

    public void Start()
    {
        bFinished = false;

        init();
        downloadUrl(_fileQueue.Dequeue());
    }

    public void downloadUrl(string url)
    {
        _curFileCount++;

        Uri uri = new Uri(url, UriKind.Absolute);

        string tmpFile = _tempPath + "/" + _curFileCount + "-" + DateTime.Now.Ticks.ToString();

        _webClient.DownloadFileAsync(uri, tmpFile, tmpFile);

		SampleDebuger.Log ("++++ downloading url: " + uri.ToString());
    }

    /// <summary>
    /// 下载完成回调
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>
    void onDownloadCompelete(object sender, AsyncCompletedEventArgs e)
    {
        if (e.Error == null && !e.Cancelled)
        {
            FileUtil.DecompressToDirectory(_upzipPath,e.UserState.ToString());
			File.Delete (e.UserState.ToString());

        }
        //else
        //{
        //    _webClient.CancelAsync();
        //    bFinished = true;
        //}


        if (_fileQueue.Count > 0)
        {
            downloadUrl(_fileQueue.Dequeue());
        }
        else
        {
            lock (AssetDownloader.WebLock)
            {
                bFinished = true;
            }
        }

    }

    /// <summary>
    /// 下载进度变化回调
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>

    private void onDownloadProcess(object sender, DownloadProgressChangedEventArgs e)
    {
        SampleDebuger.Log(string.Format("received: {0} total: ", e.BytesReceived, e.TotalBytesToReceive));
        lock (AssetDownloader.WebLock)
        {
            Loaded = e.BytesReceived;
            Total = e.TotalBytesToReceive;
        }
    }
}

#endregion