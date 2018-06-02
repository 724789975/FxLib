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
    private bool m_bDownloading = false;
    public Version m_verServerVersion;
    public EnvCheckInit m_comEnvChecker;

    public void Awake()
    {
        //_envChecker = GetComponent<EnvCheckInit>();
    }

    public void Update()
    {
        if (m_bDownloading)
        {
            lock (AssetDownloader.Instance().m_lockWeb)
            {
                if (AssetDownloader.Instance().m_bFinished)
                {
                    m_bDownloading = false;
                    m_comEnvChecker.LocalVerCheck();
                }
                else
                {
                    m_comEnvChecker.DownloadProcess(AssetDownloader.Instance().m_qwLoaded, AssetDownloader.Instance().m_qwTotal);
                }
            }
        }
    }

    /// <summary>
    /// 检查版本
    /// </summary>
    public void CheckVersionWithServer()
    {
		StartCoroutine(GetServerVersion());
    }

    IEnumerator GetServerVersion()
    {
		string szVersion = GameInstance.Instance().proServerUrl
			+ GameInstance.Instance().proServerVersion + "?" + Time.realtimeSinceStartup.ToString();
		WWW www = new WWW(szVersion);

        yield return www;
        if (!string.IsNullOrEmpty(www.error))
        {
			//无法连接资源服务器
			SampleDebuger.LogWarning ("url " + www.url + " ,error:" + www.error);
            m_comEnvChecker.GameInit();
            yield break;
        }

        if (!www.isDone)
            yield return www;

        string versionStr = www.text.Trim();
        m_verServerVersion = new Version(versionStr);
        SampleDebuger.Log(" server version = " + m_verServerVersion.ToString());
        if (VersionManager.Instance().m_verVersion.IsLower(m_verServerVersion))
        {
            StartCoroutine(CheckVersionContent());
        }
        else
        {
            //进入游戏
            m_comEnvChecker.GameInit();
        }
    }

	IEnumerator CheckVersionContent()
	{
		string szVersionContent = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerVersionContent, VersionManager.Instance().GetVersionUrl()) + "?" + Time.realtimeSinceStartup.ToString();
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
            m_comEnvChecker.StartDownload();
        }
        else
		{
			AssetBundleLoader.Instance().LoadAsset(GameObjectConstant.GetABUIPath(GameObjectConstant.g_szConfirmPanel), GameObjectConstant.g_szConfirmPanel, delegate (UnityEngine.Object ob)
				{
					GameObject go_RoleList = Instantiate((GameObject)ob, MainCanvas.Instance().transform);
					go_RoleList.GetComponent<ConfirmPanel>().Init("检测到游戏更新,请点击更新!",
						delegate (object ob1)
						{
							m_comEnvChecker.StartDownload();
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
        m_bDownloading = true;
		string szVersion = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerNextVersionPath, VersionManager.Instance().GetVersionUrl())
			+ "?" + Time.realtimeSinceStartup.ToString();
		AssetDownloader.Instance().AddURL(szVersion);
		string szVersionContent = GameInstance.Instance().proServerUrl
			+ string.Format(GameInstance.Instance().proServerVersionContent, VersionManager.Instance().GetVersionUrl())
			+ "?" + Time.realtimeSinceStartup.ToString();
		AssetDownloader.Instance().AddURL(szVersionContent);
		//AssetDownloader.Intance().AddURL(getCommonURL());
		//AssetDownloader.Intance().AddURL(getCustomURL());
		AssetDownloader.Instance().Start();
    }
}

#region 大文件下载器
public class AssetDownloader : Singleton<AssetDownloader>
{
    public object m_lockWeb = new object();//线程锁
    private WebClient m_webClient;
    private Queue<string> m_queueFiles = new Queue<string>();
    private string m_szUpZipPath;
    private string m_szTempPath;

    //status
    public bool m_bFinished = false;
    public long m_qwLoaded = 0;
    public long m_qwTotal = 0;

    public int m_dwCurFileCount = 0 ;

    public void AddURL(string url)
    {
        m_queueFiles.Enqueue(url);
    }

    public void Init()
    {
		m_szUpZipPath = Application.persistentDataPath;
        m_szTempPath = Application.temporaryCachePath;

        m_webClient = new WebClient();

        m_dwCurFileCount = 0;

        m_webClient.DownloadFileCompleted += new AsyncCompletedEventHandler(onDownloadCompelete);

        m_webClient.DownloadProgressChanged += new DownloadProgressChangedEventHandler(OnDownloadProcess);

    }

    public void Start()
    {
        m_bFinished = false;

        Init();
        DownloadUrl(m_queueFiles.Dequeue());
    }

    public void DownloadUrl(string url)
    {
        m_dwCurFileCount++;

        Uri uri = new Uri(url, UriKind.Absolute);

        string tmpFile = m_szTempPath + "/" + m_dwCurFileCount + "-" + DateTime.Now.Ticks.ToString();

        m_webClient.DownloadFileAsync(uri, tmpFile, tmpFile);

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
            FileUtil.DecompressToDirectory(m_szUpZipPath,e.UserState.ToString());
			File.Delete (e.UserState.ToString());

        }
        if (m_queueFiles.Count > 0)
        {
            DownloadUrl(m_queueFiles.Dequeue());
        }
        else
        {
            lock (AssetDownloader.Instance().m_lockWeb)
            {
                m_bFinished = true;
            }
        }
    }

    /// <summary>
    /// 下载进度变化回调
    /// </summary>
    /// <param name="sender"></param>
    /// <param name="e"></param>

    private void OnDownloadProcess(object sender, DownloadProgressChangedEventArgs e)
    {
        SampleDebuger.Log(string.Format("received: {0} total: ", e.BytesReceived, e.TotalBytesToReceive));
        lock (AssetDownloader.Instance().m_lockWeb)
        {
            m_qwLoaded = e.BytesReceived;
            m_qwTotal = e.TotalBytesToReceive;
        }
    }
}

#endregion