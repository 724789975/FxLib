using UnityEngine;
using System.Linq;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.IO;
using ICSharpCode.SharpZipLib.Zip;
using System.Runtime.Serialization.Formatters.Binary;

public class UpdateFileInfo
{
	public string filePath;
	public string md5;
	public float fileSize;

	public UpdateFileInfo (string content)
	{
		string[] one = content.Split (new string[] { "|" }, System.StringSplitOptions.RemoveEmptyEntries);
		filePath = one [0];
		md5 = one [1];
		fileSize = float.Parse (one [2]);
	}

	public bool equal (UpdateFileInfo other)
	{
		return md5.Equals (other.md5);

	}
}

public class FileUtil
{
    

	public static string getFilePath (string path)
	{
#if UNITY_EDITOR
		return Globals.streamingPath + "/" + path;
#else
        return Globals.persistenPath + "/" + path;
#endif
	}

	public static string GetAssetPath ()
	{
#if UNITY_EDITOR
		return Globals.streamingPath;
#else
        return Globals.persistenPath;
#endif
	}


    /// <summary>
    /// 读取Json文件
    /// </summary>
    /// <typeparam name="T">返回对象类型</typeparam>
    /// <param name="path">文件路径</param>
    /// <returns>返回模型对象</returns>
    public static T ReadJsonFromFile<T>(string path)
	{
		return JsonUtility.FromJson<T> (ReadTextFromFile (path));
	}


    /// <summary>
    /// 将对象模型保存为Json文件
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="path"></param>
	public static void SaveJsonToFile (object obj, string path)
	{
		string json = JsonUtility.ToJson (obj);
		SaveTextToFile (path, json);
	}


    /// <summary>
    /// 从文件中读取文本数据
    /// </summary>
    /// <param name="path">相对路径</param>
    /// <returns></returns>
    public static string ReadTextFromFile(string path)
	{
		string content = "";
		string fullPath = FileUtil.getFilePath (path);
		try {
			content = File.ReadAllText (fullPath);
		} catch {
			SampleDebuger.Log (" 没有找到文件 " + fullPath);
		}
		return content;
	}


    /// <summary>
    /// 保存文本数据保存为文件
    /// </summary>
    /// <param name="fileName">相对路径</param>
    /// <param name="text">文本数据</param>
    public static void SaveTextToFile(string path, string text)
	{
        File.WriteAllText(getFilePath(path), text);
	}


    /// <summary>
    /// 删除整个路径
    /// </summary>
    /// <param name="path"></param>
	public static void RemoveFolder (string path)
	{
        if (!Directory.Exists(path)) return;
		DirectoryInfo di = new DirectoryInfo (path);
		//删除所有文件
		FileInfo[] files = di.GetFiles ();
		for (int i = 0; i < files.Length; i++) {
			files [i].Delete ();
		}

		//删除所有目录
		DirectoryInfo[] directories = di.GetDirectories ();

		for (int i = 0; i < directories.Length; i++) {
			directories [i].Delete (true);
		}
	}


    /// <summary>
    /// 解压文件到指定路径
    /// </summary>
    /// <param name="targetPath">绝对路径</param>
    /// <param name="zipFilePath">绝对路径</param>
	public static void DecompressToDirectory (string targetPath, string zipFilePath)
	{
		if (File.Exists (zipFilePath)) {
			var compressed = File.OpenRead (zipFilePath);
			DecompressToDirectory (targetPath, compressed);
		} else {
			SampleDebuger.LogError ("Zip不存在: " + zipFilePath);
		}
	}

    /// <summary>
    /// 解压数据流到指定路径
    /// </summary>
    /// <param name="targetPath">绝对路径</param>
    /// <param name="source">数据流</param>
	public static void DecompressToDirectory (string targetPath, Stream source)
	{
		targetPath = Path.GetFullPath (targetPath);
		ZipInputStream decompressor = new ZipInputStream (source);
		ZipEntry entry;
		SampleDebuger.Log ("DecompressToDirectory:  " + targetPath);
		while ((entry = decompressor.GetNextEntry ()) != null) {
			string name = entry.Name;
			if (entry.IsDirectory && entry.Name.StartsWith ("\\"))
				name = ReplaceFirst (entry.Name, "\\", "");

			string filePath = Path.Combine (targetPath, name);
			string directoryPath = Path.GetDirectoryName (filePath);

			if (!string.IsNullOrEmpty (directoryPath) && !Directory.Exists (directoryPath))
				Directory.CreateDirectory (directoryPath);
			SampleDebuger.Log ("unziping directoryPath:  " + directoryPath);
			if (entry.IsDirectory)
				continue;

			byte[] data = new byte[2048];
			FileStream streamWriter = File.Create (filePath);
			int bytesRead;
			while ((bytesRead = decompressor.Read (data, 0, data.Length)) > 0) {
				streamWriter.Write (data, 0, bytesRead);
			}
			SampleDebuger.Log ("writing directoryPath:  " + directoryPath);
			streamWriter.Close ();
		}
		source.Close ();
	}


	/// <summary>
	/// 替换字符串中的子字符串。
	/// </summary>
	/// <param name="input">原字符串</param>
	/// <param name="oldValue">旧子字符串</param>
	/// <param name="newValue">新子字符串</param>
	/// <param name="count">替换数量</param>
	/// <param name="startAt">从第几个字符开始</param>
	/// <returns>替换后的字符串</returns>
	public static String ReplaceFirst(string input, string oldValue, string newValue, int startAt = 0)
	{
		int pos = input.IndexOf (oldValue, startAt);
		if (pos < 0) {
			return input;
		}
		return string.Concat (input.Substring (0, pos), newValue, input.Substring (pos + oldValue.Length));
	}


    /// <summary>
    /// 获得文件MD5码
    /// </summary>
    /// <param name="fs"></param>
    /// <returns></returns>
    public static string FSToMD5(FileStream fs)
    {
        try
        {
            System.Security.Cryptography.MD5 md5 = new System.Security.Cryptography.MD5CryptoServiceProvider();
            byte[] retVal = md5.ComputeHash(fs);

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < retVal.Length; i++)
            {
                sb.Append(retVal[i].ToString("x2"));
            }
            return sb.ToString();
        }
        catch (Exception ex)
        {
            throw new Exception("md5file() fail, error:" + ex.Message);
        }
    }
}
