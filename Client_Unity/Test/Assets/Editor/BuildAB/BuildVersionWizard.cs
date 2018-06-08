using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using System.Linq;
using System.IO;


public class BuildVersion
{
	public static bool bLoadAB = false;

	[MenuItem("BuildAB/DeleteAllPref ")]
	public static void cleanPlayerPref()
	{
		PlayerPrefs.DeleteAll();
	}

	[MenuItem("BuildAB/DeletePersistentPath ")]
	public static void deletePersistentPath()
	{
		Directory.Delete(Application.persistentDataPath, true);
	}

	[MenuItem("BuildAB/AdditionalIl2CppArgs")]
	public static void AdditionalIl2CppArgs()
	{
		//PlayerSettings.SetAdditionalIl2CppArgs("");
		//PlayerSettings.SetAdditionalIl2CppArgs("-O3 -g0 -DUNITY_WEBGL=1 -s PRECISE_F32=2 -s NO_EXIT_RUNTIME=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s DISABLE_EXCEPTION_CATCHING=0 -s TOTAL_MEMORY=268435456 --memory-init-file 1 --emit-symbol-map --separate-asm --output_eol linux");
		Debug.Log(PlayerSettings.GetAdditionalIl2CppArgs());
	}

	[MenuItem("BuildAB/ApiCompatibilityLevel")]
	public static void ApiCompatibilityLevel()
	{
		Debug.Log(PlayerSettings.GetApiCompatibilityLevel(EditorUserBuildSettings.selectedBuildTargetGroup).ToString());
	}

	[MenuItem("BuildAB/ApplicationIdentifier")]
	public static void ApplicationIdentifier()
	{
		//Debug.Log(EditorUserBuildSettings.selectedBuildTargetGroup.ToString());
		Debug.Log(PlayerSettings.GetApplicationIdentifier(EditorUserBuildSettings.selectedBuildTargetGroup));
	}

	[MenuItem("BuildAB/CodeStrippingLevel")]
	public static void CodeStrippingLevel()
	{
		Debug.Log(PlayerSettings.strippingLevel.ToString());
	}

	[MenuItem("BuildAB/Packager ")]
	public static void createWindow()
	{
		EditorWindow.GetWindow<BuildWindow>();
		Packager.Init();
		Packager.bLoadAB = !AssetBundleManager.SimulateAssetBundleInEditor;
	}

}

public class BuildWindow : EditorWindow
{
	public string myVersion;

	public string outPutPath = "";

	public string[] verList = new string[3] { "开发版本", "测试版本", "正式版本" };

	public static int curSelect = -1;


	void OnEnable()
	{
		myVersion = VersionEditorManager.Instance().curVersion;
		Packager.Init();
	}

	BuildTargetGroup transPlatform(TargetPlatform plat)
	{
		BuildTargetGroup ret = BuildTargetGroup.Standalone;
		switch (plat)
		{
			case TargetPlatform.IOS:
				ret = BuildTargetGroup.iOS;
				break;
			case TargetPlatform.Android:
				ret = BuildTargetGroup.Android;
				break;
			case TargetPlatform.WebGL:
				ret = BuildTargetGroup.WebGL;
				break;
		}
		return ret;
	}

	void OnGUI()
	{

		GUILayout.Label(" 指定当前版本号, 与服务器保持一致", EditorStyles.boldLabel);
		myVersion = GUILayout.TextField(myVersion);
		GUILayout.Space(20);

		// =========================== 3. 标记AB资源   ===========================
		if (GUILayout.Button("刷新版本", GUILayout.Height(30)))
		{
			string[] szAssetBundleNames = AssetDatabase.GetAllAssetBundleNames();
			for (int i = 0; i < szAssetBundleNames.Length; i++)
			{
				AssetDatabase.RemoveAssetBundleName(szAssetBundleNames[i], true);
			}

			VersionEditorManager.Instance().curVersion = myVersion;
			PlayerSettings.Android.bundleVersionCode = VersionEditorManager.Instance().getVersionNum();
			PlayerSettings.bundleVersion = myVersion;
		}

		GUILayout.Space(30);

		//=========================== 1.选择平台 ================================
		GUILayout.Label(" 选择发布平台  ", EditorStyles.boldLabel);
		GUILayout.Space(20);

		TargetPlatform select = (TargetPlatform)EditorGUILayout.EnumPopup(Packager.curTarget);
		if (select != Packager.curTarget)
		{
			// 重新判断当前版本设定
			Packager.curTarget = select;   
		}

		GUILayout.Space(20);

		// =========================== 3. 标记AB资源   ===========================
		if (GUILayout.Button("标记AB", GUILayout.Height(30)))
		{
			Packager.BuildAssetMarks();
			Packager.WritePreloadFile();
			BuildUtil.createVersion();
		}

		GUILayout.Space(20);

		if (GUILayout.Button("生成AB", GUILayout.Height(30)))
		{
			Packager.ClearABFolder();
			Packager.GenerateAB();
		}

		// =========================== 4. 是否读取AB包  ===========================
		bool cur = GUILayout.Toggle(Packager.bLoadAB, "读取AB包");

		if (Packager.bLoadAB != cur)
		{
			Packager.bLoadAB = cur;
			AssetBundleManager.SimulateAssetBundleInEditor = !Packager.bLoadAB;
		}

		GUILayout.Space(20);
		GUIContent content = new GUIContent(" 请确认完成了 AB包 的制做过程 ！！！");
		GUIStyle style = new GUIStyle();
		style.fontStyle = FontStyle.Normal;
		style.fontSize = 13;

		GUILayout.Label(content);
		GUILayout.Space(20);
		GUILayout.Label(" 选择发布版本类型:");
		GUILayout.Space(20);
		BuildTargetGroup curGroup = transPlatform(Packager.curTarget);
		string curSymbol = null;
		if (curSelect == -1)
		{
			curSymbol = PlayerSettings.GetScriptingDefineSymbolsForGroup(curGroup);
			if (curSymbol.IndexOf("RELEASE_VER", 0, curSymbol.Length) == -1){
				curSelect = 0;
			}
			else
			{
				if (curSymbol.IndexOf("STORE_VERSION", 0, curSymbol.Length) == -1)
				{
					curSelect = 1;
				}
				else
				{
					curSelect = 2;
				}
			}

		}

		int newSelect = GUILayout.SelectionGrid(curSelect, verList, 6);

		//处理不同版本的一些 PlayerSetting 设置
		if (newSelect != curSelect)
		{
			curSelect = newSelect;
			curSymbol = PlayerSettings.GetScriptingDefineSymbolsForGroup(curGroup);
			switch (curSelect)
			{
				case 0:
					{
						curSymbol = curSymbol.Replace("RELEASE_VER", "DEVELOP_VERSION");
						curSymbol = curSymbol.Replace("STORE_VERSION", "DEVELOP_VERSION");
						if (curSymbol.IndexOf("DEVELOP_VERSION", 0, curSymbol.Length) == -1)
						{
							if (!string.IsNullOrEmpty(curSymbol))
							{
								curSymbol += ";";
							}
							curSymbol += "DEVELOP_VERSION";
						}
					}
					break;

				case 1:
					{
						curSymbol = curSymbol.Replace("DEVELOP_VERSION", "RELEASE_VER");
						curSymbol = curSymbol.Replace("STORE_VERSION", "RELEASE_VER");
						if (curSymbol.IndexOf("RELEASE_VER", 0, curSymbol.Length) == -1)
						{
							if (!string.IsNullOrEmpty(curSymbol))
							{
								curSymbol += ";";
							}
							curSymbol += "RELEASE_VER";
						}
					}
					break;

				case 2:
					{
						curSymbol = curSymbol.Replace("DEVELOP_VERSION", "STORE_VERSION");
						curSymbol = curSymbol.Replace("RELEASE_VER", "STORE_VERSION");
						if (curSymbol.IndexOf("STORE_VERSION", 0, curSymbol.Length) == -1)
						{
							if (!string.IsNullOrEmpty(curSymbol))
							{
								curSymbol += ";";
							}
							curSymbol += "STORE_VERSION";
						}
					}
					break;
			}

			PlayerSettings.SetScriptingDefineSymbolsForGroup(curGroup, curSymbol);
			Debug.Log(curSymbol);
		}
		GUILayout.Space(20);
		// =========================== 5. 生成安装包    ===========================
		if (GUILayout.Button("拷贝资源 ", GUILayout.Height(30)))
		{

			switch (Packager.curTarget)
			{
				case TargetPlatform.IOS:
					BuildUtil.copyPlatformRes(BuildTarget.iOS);
					break;
				case TargetPlatform.Windows:
					BuildUtil.copyPlatformRes(BuildTarget.StandaloneWindows);
					break;
				case TargetPlatform.Android:
					BuildUtil.copyPlatformRes(BuildTarget.Android);
					break;
				case TargetPlatform.WebGL:
					BuildUtil.copyPlatformRes(BuildTarget.WebGL);
					break;
			}
			AssetDatabase.Refresh();
		}
		GUILayout.Space(20);
		// =========================== 5. 生成安装包    ===========================
		if (GUILayout.Button("生成安装包 ", GUILayout.Height(30)))
		{
			if (myVersion.Length == 0 || myVersion.Equals("0.0.0"))
			{
				EditorUtility.DisplayDialog(" Error ！！", " 请修改版本为有效数字", "确定");
			}
			else
			{
				switch (Packager.curTarget)
				{
					case TargetPlatform.IOS:
						BuildUtil.buildIOS();
						break;
					case TargetPlatform.Windows:
						BuildUtil.buildWindows();
						break;
					case TargetPlatform.Android:
						BuildUtil.buildAndroid();
						break;
					case TargetPlatform.WebGL:
						BuildUtil.buildWebGL();
						break;
				}
			}
		};
		GUILayout.Space(20);

		if (Packager.curTarget == TargetPlatform.IOS)
		{

			if (GUILayout.Button("生成IPA", GUILayout.Height(30)))
			{
				IPABuilder.buildIPA();

			}
			GUILayout.Space(20);

		}

		if (GUILayout.Button("生成版本更新包 ", GUILayout.Height(30)))
		{
			BuildUtil.PatchAll ();
		}
	}
}

public class BuildUtil
{
	static string[] levels = { "Assets/Scene/launcher.unity" };

	static public string getPath() 
	{
		int tmp = Application.dataPath.LastIndexOf("/");
		string path = Application.dataPath.Substring(0, tmp)+"/build";
		return path;
	}

	static public void buildIOS()
	{
		BuildTarget type = BuildTarget.iOS;
		copyWWise(type);
		copyABRes(type);
		//createVersion();
		AssetDatabase.Refresh();
		BuildPipeline.BuildPlayer(levels, BuildUtil.getPath() + "/proj_ios", BuildTarget.iOS, BuildOptions.Il2CPP | BuildOptions.ShowBuiltPlayer);
	}

	static public void buildAndroid()
	{
		BuildTarget type = BuildTarget.Android;
		copyWWise(type);
		copyABRes(type);
		//createVersion();
		AssetDatabase.Refresh();
		PlayerSettings.Android.keystoreName = Application.dataPath + "/../SDK/user.keystore";
		PlayerSettings.Android.keyaliasPass = "123456";

		PlayerSettings.Android.keyaliasName = "star";
		PlayerSettings.Android.keystorePass = "123456";
		PlayerSettings.Android.bundleVersionCode = VersionEditorManager.Instance().getVersionNum();
		BuildPipeline.BuildPlayer(levels, BuildUtil.getPath() + "/proj.apk", BuildTarget.Android, BuildOptions.ShowBuiltPlayer);

	}

	static public void buildWindows()
	{
		BuildTarget type = BuildTarget.StandaloneWindows;
		copyWWise(type);
		copyABRes(type);
		//createVersion();
		AssetDatabase.Refresh();
		BuildPipeline.BuildPlayer(levels, BuildUtil.getPath() + "/proj_win/game.exe", BuildTarget.StandaloneWindows, BuildOptions.ShowBuiltPlayer | BuildOptions.Development);
	}

	static public void buildWebGL()
	{
		AssetDatabase.Refresh();
		string sourcePath = Application.dataPath + "/Resources";
		DirectoryInfo folder = new DirectoryInfo(sourcePath + "/screen");
		FileSystemInfo[] files = folder.GetFileSystemInfos();
		int length = files.Length;
		for (int i = 0; i < length; i++)
		{
			if(!files[i].Name.Contains(".meta"))
			{
				string[] levels = new string[1];
				levels[0] = files[i].FullName;
				string szOut = Packager.GetABPath() + "/Assets/Resources/screen/" + files[i].Name;
				szOut = szOut.Replace(files[i].Extension, "");
				BuildPipeline.BuildPlayer(levels, szOut.ToLower(), BuildTarget.WebGL, BuildOptions.BuildAdditionalStreamedScenes | BuildOptions.AllowDebugging | BuildOptions.UncompressedAssetBundle | BuildOptions.BuildScriptsOnly);
			}
		}
		BuildTarget type = BuildTarget.WebGL;
		//copyWWise(type);
		copyABRes(type);
		//createVersion();
		BuildPipeline.BuildPlayer(levels, Application.dataPath + "/../view", BuildTarget.WebGL, BuildOptions.ShowBuiltPlayer | BuildOptions.AllowDebugging | BuildOptions.BuildScriptsOnly);
	}

	static public void copyPlatformRes(BuildTarget os)
	{
		//copyWWise(os);
		copyABRes(os);
	}

	static public void cleanABPath()
	{
		deleteDirectroy(Application.streamingAssetsPath + "/AssetBundles");
	}

	static public void copyABRes(BuildTarget os)
	{
		cleanABPath();
		string osPath = getPlatformDir(os);
		_copyDirectory(Application.dataPath + "/../AssetBundles/" + osPath, 
				Application.streamingAssetsPath + "/AssetBundles/" + osPath,
				new string[] { ".manifest", ".meta"},
				new string[] { osPath + ".manifest" });
	}

	static public void copyFullABRes()
	{
		cleanABPath();
		copyDirectory(Application.dataPath + "/../AssetBundles",  Application.streamingAssetsPath + "/AssetBundles");
	}

	static public void cleanWWise()
	{
		deleteDirectroy(Application.streamingAssetsPath + "/Audio/GeneratedSoundBanks");
	}

	static public void copyWWise(BuildTarget os)
	{
		cleanWWise();
		string osPath = getPlatformDir(os);
		copyDirectory(Application.dataPath + "/Wwise/Audio/GeneratedSoundBanks/" + osPath, Application.streamingAssetsPath + "/Audio/GeneratedSoundBanks/" + osPath);
	}

	static public void copyFullWWise()
	{
		cleanWWise();
		copyDirectory(Application.dataPath + "/Wwise/Audio/GeneratedSoundBanks/",   Application.streamingAssetsPath + "/Audio/GeneratedSoundBanks/");
	}

	static public string getPlatformDir(BuildTarget os){
		switch (os)
		{
			case BuildTarget.Android:
				return "Android";
			case BuildTarget.iOS:
				return "iOS";
			case BuildTarget.StandaloneWindows:
			case BuildTarget.StandaloneWindows64:
				return "Windows";
			case BuildTarget.WebGL:
				return "WebGL";
			default:
				return null;
		}
	}

	static public string getPlatformManifest()
	{
		return getPlatformDir(Packager.getBuildTarget()) + ".manifest";

	}


	static public void deleteDirectroy(string dirName)
	{
		DirectoryInfo d = new DirectoryInfo(dirName);
		if (d.Exists)
		{
			Directory.Delete(dirName, true);
		}
	}

	static public void copyDirectory(string fromDir, string toDir)
	{
		_copyDirectory(fromDir, toDir);
	}

	static public void _copyDirectory(string fromDir, string toDir, string[] ignoreExts = null, string[] needFiles = null)
	{
		if (Directory.Exists(fromDir))
		{
			if (!Directory.Exists(toDir))
			{
				Directory.CreateDirectory(toDir);
			}
			string[] files = Directory.GetFiles(fromDir, "*", SearchOption.AllDirectories);
			string[] dirs = Directory.GetDirectories(fromDir, "*", SearchOption.AllDirectories);
			foreach (string soureDir in dirs)
			{
				string desDir = soureDir.Replace(fromDir, toDir);
				Debug.Log("path: " + desDir);
				if (!Directory.Exists(desDir))
				{
					Directory.CreateDirectory(desDir);
				}
			}

			foreach (string soureFile in files)
			{
				string extName = Path.GetExtension(soureFile);
				string fileName = Path.GetFileName(soureFile);
				if (needFiles != null && needFiles.Contains<string>(fileName))
				{
					File.Copy(soureFile, soureFile.Replace(fromDir, toDir), true);

				}
				else if (!string.IsNullOrEmpty(extName) && ignoreExts != null && ignoreExts.Contains<string>(extName))
				{
					Debug.Log("ignoreFile: " + soureFile);

				}
				else
				{
					File.Copy(soureFile, soureFile.Replace(fromDir, toDir), true);
				}
			}
		}
	}

	static public void createVersion()
	{
		string streamPath = Application.streamingAssetsPath;

		FileInfo fi = new FileInfo(streamPath + "/streamPath.txt");
		using (StreamWriter sw = fi.CreateText())
		{
			getFilePath(streamPath, sw);
		}

		AssetDatabase.Refresh();
		Debug.Log("新版本生成成功,  ");
	}

	static public void getFilePath(string sourcePath, StreamWriter sw)
	{

		DirectoryInfo info = new DirectoryInfo(sourcePath);

		foreach (FileSystemInfo fsi in info.GetFileSystemInfos())
		{

			if (fsi.Extension != ".meta" && fsi.Name != "streamPath.txt")
			{

				string[] r = fsi.FullName.Split(new string[] { "StreamingAssets" }, System.StringSplitOptions.None); //得到相对路径

				r[1] = r[1].Replace('\\', '/'); //安卓上只能识别"/"

				if (fsi is DirectoryInfo)
				{ //是文件夹则迭代

					sw.WriteLine(r[1] + " | 0"); //按行写入
					bool ignored = fsi.FullName.EndsWith("AssetBundles");
					if (!ignored)
					{
						getFilePath(fsi.FullName, sw);
					}

				}
				else
				{
					sw.WriteLine(r[1] + " | 1" + "|" + string.Format("{0:F}", ((FileInfo)fsi).Length / 1024.0f)); //按行写入
				}
			}
		}
	}

	static public string getVersionNum()
	{
		string streamPath = Application.streamingAssetsPath;
		byte[] ret = File.ReadAllBytes(streamPath + "/version.txt");
		if (ret == null || ret.Length == 0)
			return "1.0.0";
		string versionNum = System.Text.Encoding.Default.GetString(ret);
		return versionNum;
	}

	static public void SwitchToAndroid()
	{
		EditorUserBuildSettings.SwitchActiveBuildTarget (BuildTargetGroup.Android, BuildTarget.Android);
	}

	static public void PatchAll(){
		BuildUtil.copyFullABRes ();
		BuildUtil.copyFullWWise();
		PatchUtil.Instance().init();
		PatchUtil.Instance().buildPatch();

	}
}

