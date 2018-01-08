using UnityEngine;
using System;
using System.IO;
using System.Text;

public class Utility
{
    /// <summary>
    /// "将c# unicode字符串转换为服务器utf8字符串，并写入流中"
    /// </summary>
    /// <param name="str">"要转换的字符串"</param>
    /// <param name="bw">"要写入的流"</param>
    static public void Write(string str, BinaryWriter bw, bool bUnicode)
    {
        string temp = "";
        if (str != null)
        {
            temp = str;
        }


        byte[] bufs;
        if (bUnicode)
        {
            bufs = Encoding.Unicode.GetBytes(temp);
        }
        else
        {
            bufs = Encoding.UTF8.GetBytes(temp);
        }

        ushort length = (ushort)bufs.Length;
        bw.Write(length);
        if (length > 0)
            bw.Write(bufs);
    }

    /// <summary>
    /// "将服务器的utf8字符串转换为c# unicode字符串"
    /// </summary>
    /// <param name="br">"读取的流"</param>
    /// <returns>"返回的c#的字符串"</returns>
    static public string ReadString(BinaryReader br, bool bUnicode)
    {
        ushort length = br.ReadUInt16();
        if (length > 0)
        {
            byte[] bufs = br.ReadBytes(length);

            string result = "";
            if (bUnicode)
            {
                result = Encoding.Unicode.GetString(bufs);
            }
            else
            {
                result = Encoding.UTF8.GetString(bufs);
            }
            return result;
        }
        else
        {
            return "";
        }
    }

}


public class Common
{

    public static float TimeScale
    {
        set
        {
            Time.timeScale = value;
        }
        get
        {
            return Time.timeScale;
        }
    }

    private static float m_copyTimeScale = 1;
    public static float CopyTimeScale
    {
        get { return m_copyTimeScale; }
        set { m_copyTimeScale = value; }
    }

    public static string CurrTimeString
    {
        get
        {
            return DateTime.Now.Month.ToString() + "_"
                + DateTime.Now.Day.ToString() + "_"
                + DateTime.Now.Hour.ToString() + "_"
                + DateTime.Now.Minute.ToString() + "_"
                + DateTime.Now.Second.ToString();
        }
    }
    //根据名称关键字(名字的一部分)查找子物体
    public static GameObject findChildObjPartName(GameObject root, string partName, bool includeinactive = false)
    {
        return SysUtil.findChildObjPartName(root, partName, includeinactive);
    }

    public static float GetScreenScale()
    {
        float _basePre = Config.STANDARD_WIDTH / Config.STANDARD_HEIGHT;
        float _realPre = (float)Screen.width / (float)Screen.height;
        float _screenScale = 1.0f;
        if (_basePre > _realPre)
        {
            float _w = Config.STANDARD_WIDTH / (float)Screen.width;
            float _h = Config.STANDARD_HEIGHT / (float)Screen.height;
            if (_w >= 1.0f && _h >= 1.0f)
            {
                if (_w > _h)
                {
                    _screenScale = _w;
                }
                else
                {
                    _screenScale = _h;
                }
            }
            else
            {
                _screenScale = (float)Screen.width / (float)Screen.height;
            }
        }
        else
        {
            _screenScale = 1.0f;
        }

        return _screenScale;
    }
}

