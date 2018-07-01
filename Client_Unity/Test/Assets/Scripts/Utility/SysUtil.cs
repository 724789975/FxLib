using UnityEngine;
using UnityEngine.SceneManagement;
using System.Collections;
using System.IO;
using System.Text;
using System;
#if UNITY_EDITOR
using UnityEditor;
#endif

/// <summary>
/// =============================== 系统工具类 ===============================
/// </summary>
// 

public class SysUtil
{

    /// <summary>
    /// 将枚举转换为Int
    /// </summary>
    /// <param name="obj"></param>
    /// <returns></returns>
    public static int GetEnumWithInt(object obj)
    {
        if (obj.GetType().IsEnum)
        {
            return (int)obj;
        }
        return 0;
    }


    /// <summary>
    /// 获得当前平台的字符名称
    /// </summary>
    /// <returns></returns>
    public static string GetPlatformName()
    {
#if UNITY_EDITOR
		switch (EditorUserBuildSettings.activeBuildTarget)
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
#else
		RuntimePlatform platform = Application.platform;
        switch (platform)
        {
            case RuntimePlatform.Android:
                return "Android";
            case RuntimePlatform.IPhonePlayer:
                return "iOS";
            case RuntimePlatform.WindowsEditor:
            case RuntimePlatform.WindowsPlayer:
                return "Windows";
            case RuntimePlatform.WebGLPlayer:
                return "WebGL";
            default:
                return null;
        }
#endif
	}


    /// <summary>
    /// 根据名字获得子节点
    /// </summary>
    /// <param name="root">根节点</param>
    /// <param name="name">名称</param>
    /// <returns>子节点</returns>
    public static GameObject FindChild(GameObject root, string name)
    {

        foreach (Transform tx in root.GetComponentsInChildren<Transform>(true))
        {
            if (tx.name == name)
            {
                return tx.gameObject;
            }
        }
        return null;
    }

    //根据名称关键字(名字的一部分)查找子物体
    public static GameObject findChildObjPartName(GameObject root, string partName, bool includeinactive = false)
    {
        foreach (Transform tx in root.GetComponentsInChildren<Transform>(includeinactive))
        {
            if (tx.name.Contains(partName))
            {
                return tx.gameObject;
            }
        }
        return null;
    }

    public static bool CompareUInt64(UInt64 one, UInt64 two)
    {
        return (one > two);
    }

    public static bool EqualsUInt64(UInt64 one, UInt64 two)
    {
        return (one == two);
    }

    public static double GetMillisecond()
    {
        TimeSpan now = DateTime.Now - new DateTime(1970, 1, 1);
        return now.TotalSeconds;
    }

    /// <summary>
    /// 设置物体的层;
    /// </summary>
    public static void SetLayer(GameObject go, string layerName)
    {
        int layer = LayerMask.NameToLayer(layerName);
        SetLayer(go, layer);
    }
    public static void SetLayer(GameObject go, int layer)
    {
        go.layer = layer;

        foreach (Transform child in go.transform)
        {
            SetLayer(child.gameObject, layer);
        }
    }

	public static void SendMessage(SessionObject pSessionObject, Google.Protobuf.IMessage pMsg, string szName)
	{
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString(szName);
		byte[] pProto = new byte[pMsg.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		pMsg.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		pSessionObject.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public static string GetScesneNameBySceneState(GameProto.EGameSceneState eState)
	{
		string szName = "";
		switch (eState)
		{
			case GameProto.EGameSceneState.EssPrepare:
				{
					szName = GameConstant.g_szGamePrepareScene;
				}
				break;
			case GameProto.EGameSceneState.EssGaming:
				{
					szName = GameConstant.g_szGameScene;
				}
				break;
			case GameProto.EGameSceneState.EssGameReady:
				{
					szName = GameConstant.g_szGameScene;
				}
				break;
			case GameProto.EGameSceneState.EssTransact:
				{
					szName = GameConstant.g_szGameScene;
				}
				break;
		}

		return szName;
	}

}
