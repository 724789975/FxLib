using UnityEditor;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;

public enum TargetPlatform{
    IOS,
	Windows,
    Android,
	WebGL,
}

public class Packager
{
    public static string platform = string.Empty;

    public static bool bLoadAB = false;

    public static bool bNeedIPA = false;

    private static bool bAssetBundle = true;

    public static TargetPlatform curTarget = TargetPlatform.IOS;

    public static void Init() 
    {
		// 判断当前所在平台
		switch (EditorUserBuildSettings.activeBuildTarget)
		{
			case BuildTarget.iOS:
				Packager.curTarget = TargetPlatform.IOS;
				break;
			case BuildTarget.Android:
				Packager.curTarget = TargetPlatform.Android;
				break;
			case BuildTarget.StandaloneWindows:
			case BuildTarget.StandaloneWindows64:
				Packager.curTarget = TargetPlatform.Windows;
				break;
			case BuildTarget.WebGL:
				Packager.curTarget = TargetPlatform.WebGL;
				break;
			default:
				Packager.curTarget = TargetPlatform.Windows;
				break;
		}
    }

    public static BuildTarget getBuildTarget()
    {
		BuildTarget target = BuildTarget.StandaloneWindows;
        switch (curTarget)
        {
            case TargetPlatform.IOS:
                target = BuildTarget.iOS;
                break;

            case TargetPlatform.Android:
                target = BuildTarget.Android;
                break;

			case TargetPlatform.Windows:
                target = BuildTarget.StandaloneWindows64;
                break;
			case TargetPlatform.WebGL:
				target = BuildTarget.WebGL;
				break;
        }
        return target;
    }

    /// <summary>
    /// 生成绑定素材
    /// </summary>
    public static void BuildAssetResource()
    {
        //1.生成AB包
        Packager.GenerateAB();

		////2.复制 lua 文件
		//string luaDataPath = Application.dataPath + "/StreamingAssets/";
		//Recursive(luaDataPath);
		//int n = 0;
		//foreach (string f in files)
		//{
		//	if (f.EndsWith(".meta")) continue;
		//	string newfile = f.Replace(luaDataPath, "");
		//	string newpath = resPath + "/" + newfile;
		//	string path = Path.GetDirectoryName(newpath);
		//	if (!Directory.Exists(path)) Directory.CreateDirectory(path);

		//	if (File.Exists(newpath))
		//	{
		//		File.Delete(newpath);
		//	}
		//	if (Packager.bEncodeLua)
		//	{
		//		UpdateProgress(n++, files.Count, newpath);
		//		EncodeLuaFile(f, newpath, isWin);
		//	}
		//	else
		//	{
		//		File.Copy(f, newpath, true);
		//	}
		//}
		//EditorUtility.ClearProgressBar();
	}

	public static string GetABPath()
	{
		return Application.dataPath + "/../AssetBundles/" + Packager.GetPlatformFolderForAssetBundles(EditorUserBuildSettings.activeBuildTarget);
	}

    //生成AB包
    public static void GenerateAB()
    {
		string resPath = GetABPath();
        if (!Directory.Exists(resPath))
		{
            Directory.CreateDirectory(resPath);
		}

        if (Packager.bAssetBundle)
		{
			SampleDebuger.Log("build : " + resPath);
			BuildPipeline.BuildAssetBundles(resPath, BuildAssetBundleOptions.UncompressedAssetBundle | BuildAssetBundleOptions.ForceRebuildAssetBundle, EditorUserBuildSettings.activeBuildTarget);
		}
		AssetDatabase.Refresh();
    }

    //清除AB包
    public static void ClearABFolder()
    {
		string resPath = GetABPath();
        if (Directory.Exists(resPath))
        {
            Directory.Delete(resPath, true);
        }
    }

    /// <summary>
    /// 数据目录
    /// </summary>
    static string AppDataPath
    {
        get { return Application.dataPath.ToLower(); }
    }

    /// <summary>
    /// 遍历目录及其子目录
    /// </summary>
    static void Recursive(string path)
    {
        string[] names = Directory.GetFiles(path);
        string[] dirs = Directory.GetDirectories(path);
        foreach (string filename in names)
        {
            string ext = Path.GetExtension(filename);
            if (ext.Equals(".meta")) continue;
        }
        foreach (string dir in dirs)
        {
            Recursive(dir);
        }
    }

    public static void UpdateProgress(int progress, int progressMax, string desc)
    {
        string title = "Processing...[" + progress + " - " + progressMax + "]";
        float value = (float)progress / (float)progressMax;
        EditorUtility.DisplayProgressBar(title, desc, value);
    }

    static void EncodeLuaFile(string srcFile, string outFile, bool isWin)
    {
        if (!srcFile.ToLower().EndsWith(".lua"))
        {
            File.Copy(srcFile, outFile, true);
            return;
        }
        string luaexe = string.Empty;
        string args = string.Empty;
        string exedir = string.Empty;
        string currDir = Directory.GetCurrentDirectory();
        if (Application.platform == RuntimePlatform.WindowsEditor)
        {
            luaexe = "luajit.exe";
            args = "-b " + srcFile + " " + outFile;
            exedir = AppDataPath + "/../../Tools/LuaEncoder/luajit/";
        }
        else if (Application.platform == RuntimePlatform.OSXEditor)
        {
            luaexe = "./luac";
            args = "-o " + outFile + " " + srcFile;
            exedir = AppDataPath + "/../../Tools/LuaEncoder/luavm/";
        }
        Directory.SetCurrentDirectory(exedir);
        ProcessStartInfo info = new ProcessStartInfo();
        info.FileName = luaexe;
        info.Arguments = args;
        info.WindowStyle = ProcessWindowStyle.Hidden;
        info.UseShellExecute = isWin;
        info.ErrorDialog = true;

        Process pro = Process.Start(info);
        pro.WaitForExit();
        Directory.SetCurrentDirectory(currDir);
    }

    public static string GetPlatformFolderForAssetBundles(BuildTarget target)
    {
		switch (target)
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
			// Add more build targets for your own.
			// If you add more targets, don't forget to add the same platforms to GetPlatformFolderForAssetBundles(RuntimePlatform) function.
			default:
				return null;
		}
	}

    public static void BuildAssetMarks()
    {
        string sourcePath = Application.dataPath + "/Resources";
        List<string> fileSystemEntries = new List<string>();

        fileSystemEntries
            .AddRange(Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories)
                          .Select(d => d + "\\"));
        fileSystemEntries
            .AddRange(Directory.GetFiles(sourcePath, "*", SearchOption.AllDirectories));

        for (int i = 0; i < fileSystemEntries.Count; i++ )
        {
            string filePath = fileSystemEntries[i];
            string ext = Path.GetExtension(filePath).ToLower();

			string assetPath = filePath.Replace(Application.dataPath, "Assets");
            AssetImporter import = AssetImporter.GetAtPath(assetPath);
            if (import != null)
            {
                if (ext.Equals(".prefab") || ext.Equals(".png") || ext.Equals(".mat") || ext.Equals(".unity") || ext.Equals(".anim") || ext.Equals(".shader"))
				{
					if (!ext.Equals(".png"))
                    {
                        assetPath = assetPath.Replace(ext, "");
                    }
                    else
                    {
                        assetPath = assetPath.Replace(ext, ".img");
                    }
                    import.assetBundleName = assetPath;
                }
                else
                {
                    import.assetBundleName = "";
                }
                UpdateProgress(i, fileSystemEntries.Count, assetPath);
            }
        }
        EditorUtility.ClearProgressBar();
    }

    public static void WritePreloadFile()
    {
        string sourcePath = Application.dataPath + "/Resources";
        List<string> fileSystemEntries = new List<string>();

        fileSystemEntries.AddRange(Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories).Select(d => d + "\\"));
        fileSystemEntries.AddRange(Directory.GetFiles(sourcePath, "*", SearchOption.AllDirectories));
		List<string> fileList = new List<string>();
        for (int i = 0; i < fileSystemEntries.Count; i++)
        {
            string filePath = fileSystemEntries[i];
            string ext = Path.GetExtension(filePath).ToLower();

            string assetPath = filePath.Replace(Application.dataPath, "Assets");
            AssetImporter import = AssetImporter.GetAtPath(assetPath);
            if (import != null)
            {
                assetPath = assetPath.Replace(ext, "").Replace("\\", "/");
                if (ext.Equals(".prefab")&&assetPath.Contains("Common"))
                {
					if (!fileList.Contains(assetPath))
					{
						fileList.Add(assetPath);
					}
                }

                UpdateProgress(i, fileSystemEntries.Count, assetPath);
            }
        }

        string json = JsonUtility.ToJson(fileList, true);

        File.WriteAllText(Application.streamingAssetsPath + "/Config/PreloadAB.json", json);

        EditorUtility.ClearProgressBar();        
    }
}

