using UnityEngine;
using System;
using System.IO;
using System.Text;

public class Utility
{
    /// <summary>
    /// "��c# unicode�ַ���ת��Ϊ������utf8�ַ�������д������"
    /// </summary>
    /// <param name="str">"Ҫת�����ַ���"</param>
    /// <param name="bw">"Ҫд�����"</param>
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
    /// "����������utf8�ַ���ת��Ϊc# unicode�ַ���"
    /// </summary>
    /// <param name="br">"��ȡ����"</param>
    /// <returns>"���ص�c#���ַ���"</returns>
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
    //�������ƹؼ���(���ֵ�һ����)����������
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

