using ICSharpCode.SharpZipLib.Zip;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using System.Linq;
using System.IO;

/// <summary>
/// 补丁生成工具
/// </summary>
public class PatchUtil : Singleton<PatchUtil>
{
	string sourcePath = Application.streamingAssetsPath;//源文件路径
	string targetPath = Application.dataPath + "/../patch/";//补丁存放路径
	List<PatchInfo> patchList = new List<PatchInfo>();

	public void init()
	{
		patchList.Clear();
		updatePatchList();
	}

	void updatePatchList()
	{
		string[] foldList = Directory.GetDirectories(targetPath);
		for (int i = 0; i < foldList.Length; i++)
		{
			string folderName = foldList[i];
			string vername = Path.GetFileName(folderName).Replace("_", ".");
			PatchInfo info = new PatchInfo(vername);
			info.loadContent(folderName);
			if (info.isVaild)
			{
				patchList.Add(info);
			}
		}
	}

	public void buildPatch()
	{
		//1. 创建当前版本目录
		string folderName = VersionEditorManager.Instance().curVersion.Replace(".", "_");
		folderName = targetPath + folderName;
		if (Directory.Exists(folderName)) return;

		PatchInfo patchInfo = new PatchInfo(VersionEditorManager.Instance().curVersion);
		Directory.CreateDirectory(folderName);

		//2. 统计当前版本所有文件信息，保存至文本文件
		List<string> fileSystemEntries = new List<string>();

		fileSystemEntries.AddRange(Directory.GetFiles(sourcePath, "*", SearchOption.AllDirectories));

		FileStream fs = new FileStream(folderName + "/files.txt", FileMode.CreateNew);
		StreamWriter sw = new StreamWriter(fs);

		for (int i = 0; i < fileSystemEntries.Count; i++)
		{
			string file = fileSystemEntries[i];
			file = file.Replace("\\", "/");

			if (file.EndsWith(".meta") || file.Contains(".DS_Store") || (file.Contains(".manifest")&&!(file.Contains(BuildUtil.getPlatformManifest()) ))) continue;

			FileStream fileStream = new FileStream(file, FileMode.Open);

			int size = (int)fileStream.Length;

			string md5 = FileUtil.FSToMD5(fileStream);
			string value = file.Replace(sourcePath, string.Empty).Substring(1);
			string content = value + "|" + md5 + "|" + size;
			patchInfo.addFileInfo(content);
			sw.WriteLine(content);
			fileStream.Close();
			Packager.UpdateProgress(i, fileSystemEntries.Count, "Generating file list..");
		}
		sw.Close(); fs.Close();

		//3.与历史版本对比压缩所有差异文件
		foreach (PatchInfo pInfo in patchList)
		{
			ArrayList diffFiles = pInfo.getDiffFiles(patchInfo);
			if (diffFiles.Count == 0) continue;

			FileStream commonStream = new FileStream(pInfo.getPatchPath() + "/Common.zip", FileMode.Create);
			ZipOutputStream commonZipper = new ZipOutputStream(commonStream);
			commonZipper.SetLevel(5);

			FileStream iosStream = new FileStream(pInfo.getPatchPath() + "/iOS.zip", FileMode.Create);
			ZipOutputStream iosZipper = new ZipOutputStream(iosStream);
			iosZipper.SetLevel(5);

			ZipOutputStream androidZipper = new ZipOutputStream(new FileStream(pInfo.getPatchPath() + "/Android.zip", FileMode.Create));
			androidZipper.SetLevel(5);
			ZipOutputStream winZipper = new ZipOutputStream(new FileStream(pInfo.getPatchPath() + "/Windows.zip", FileMode.Create));
			winZipper.SetLevel(5);

			string versionNum = pInfo.ver.proCurVersion;
			for (int i = 0; i < diffFiles.Count; i++)
			{
				string fileName = diffFiles[i] as string;
				ZipOutputStream compressor = commonZipper;
				if (fileName.Contains("AssetBundles/iOS/")|| fileName.Contains("Audio/GeneratedSoundBanks/iOS/"))
				{
					compressor = iosZipper;
				}
				else if(fileName.Contains("AssetBundles/Windows/") || fileName.Contains("Audio/GeneratedSoundBanks/Windows/"))
				{
					compressor = winZipper;
				}
				else if(fileName.Contains("AssetBundles/Android/") || fileName.Contains("Audio/GeneratedSoundBanks/Android/"))
				{
					compressor = androidZipper;
				}
				compressor.PutNextEntry(new ZipEntry(fileName));
				string fullPath = sourcePath + "/" + fileName;
				Packager.UpdateProgress(i, diffFiles.Count, " Compress version: " + versionNum + " on file: " + fileName);
				byte[] data = new byte[2048];
				using (FileStream input = File.OpenRead(fullPath))
				{
					int bytesRead;
					while ((bytesRead = input.Read(data, 0, data.Length)) > 0)
					{
						compressor.Write(data, 0, bytesRead);
					}
				}
			}
			commonZipper.Finish();
			iosZipper.Finish();
			androidZipper.Finish();
			winZipper.Finish();
		}

		//4. 记录当前版本号
		VersionEditorManager.Instance().saveVersion(targetPath + "version.txt");


		fs = new FileStream(folderName + "/mark.txt", FileMode.CreateNew);
		fs.Close ();
		EditorUtility.ClearProgressBar();
	}
}

/// <summary>
/// 补丁包信息
/// </summary>
public class PatchInfo
{
	public Dictionary<string, UpdateFileInfo> fileList = new Dictionary<string, UpdateFileInfo>();
	public Version ver = null;
	public bool isVaild = false;
	public string storePath = "";

	public PatchInfo(string name)
	{
		ver = new Version(name);
	}

	//从文本文件读取信息
	public void loadContent(string path)
	{
		fileList.Clear();
		string ret = null;
		path += "/files.txt";
		try
		{
			ret = File.ReadAllText(path);
			string[] fileContent = ret.Split(new string[] { "\n" }, System.StringSplitOptions.RemoveEmptyEntries);
			for (int i = 0; i < fileContent.Length; i++)
			{
				UpdateFileInfo fileInfo = new UpdateFileInfo(fileContent[i]);
				fileList[fileInfo.filePath] = fileInfo;
			}
			isVaild = true;
		}
		catch
		{
			Debug.Log(" 没有找到文件 files.txt");
		}
		storePath = Path.GetDirectoryName(path);
	}

	//添加文件信息
	public void addFileInfo(string content)
	{
		UpdateFileInfo info = new UpdateFileInfo(content);
		fileList[info.filePath] = info;
	}

	//获取差异文件
	public ArrayList getDiffFiles(PatchInfo other)
	{
		ArrayList list = new ArrayList();
		foreach (KeyValuePair<string, UpdateFileInfo> info in other.fileList)
		{
			string key = info.Key;
			UpdateFileInfo newInfo = info.Value;
			UpdateFileInfo curInfo = null;
			fileList.TryGetValue(key, out curInfo);
			if (curInfo != null && curInfo.equal(newInfo))
				continue;
			list.Add(key);
		}
		return list;
	}

	//获取补丁路径
	public string getPatchPath()
	{
		return storePath;
	}
}