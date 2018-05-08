using System;
using UnityEditor;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using System.IO;


public class GetObjectPath 
{
    private const string FILTER_STRING_PREMIX = "UI_";

    [MenuItem( "GameObject/ObjectPath %Q" )]
    public static void GetSelectUIObjectListPath()
    {
        string strResult = "";
        var goArr = Selection.gameObjects;
        for( int i = 0; i < goArr.Length; i++ )
        {
            string strPre = i == 0 ? "" : "\t";
            string strEnd = i == goArr.Length - 1 ? "" : "\n";
            strResult += strPre+GetObjectPath.GetLuaCode( goArr[i] ) + strEnd;
        }

        //复制reuqire lua 文件
        UnityEngine.Object[] selection = Selection.GetFiltered(typeof(UnityEngine.Object), SelectionMode.Assets);

        if (string.IsNullOrEmpty(strResult))
        {
            foreach (UnityEngine.Object obj in selection)
            {
                strResult = AssetDatabase.GetAssetPath(obj);
                if (!string.IsNullOrEmpty(strResult) && File.Exists(strResult))
                {
                    strResult = strResult.Replace(".lua", "\"");
                    strResult = strResult.Replace("Assets/StreamingAssets/Lua/", "require \"");
                    break;
                }
            }
        }

        GetObjectPath.SaveToClipBoard( strResult ); 
        Debug.Log( strResult );
    }

	[MenuItem("Assets/Print Asset Path")]
	private static void PrintSelected()
	{
		foreach (UnityEngine.Object obj in Selection.objects)
		{
			if (AssetDatabase.Contains(obj))
			{
				Debug.Log(string.Format("{0} ({1})", AssetDatabase.GetAssetPath(obj), obj.GetType()));
			}
			else
			{
				Debug.LogWarning(string.Format("{0} is not a source asset.", obj));
			}
		}
	}


    public static void GetUIObjectPath()
    {
        if( Selection.activeObject != null )
        {
            GameObject go = Selection.activeObject as GameObject;
            string strResult = GetObjectPath.GetLuaCode( go );
            GetObjectPath.SaveToClipBoard( strResult );
            Debug.Log( strResult );
        }
    }

    private static string GetLuaCode(GameObject go)
    {
        string strPath = GetObjectPath.GetGameObjectPath( go );
        Type type = GetObjectPath.GetGameObjectTypeDefine( go );

        string valueName = "_"+ char.ToLower(go.name[0]) + go.name.Substring(1);

        strPath = "{ \"" + valueName + type.Name + "\" , \"" + strPath + "\"},";

        return strPath;
    }

    private static Type GetGameObjectTypeDefine(GameObject go)
    {
        if( go.GetComponent<ScrollRect>() != null )
            return typeof( ScrollRect );
        if( go.GetComponent<InputField>() != null )
            return typeof( InputField );
        if( go.GetComponent<ToggleGroup>() != null )
            return typeof( ToggleGroup );
        if( go.GetComponent<Toggle>() != null )
            return typeof( Toggle );
        if( go.GetComponent<Button>() != null )
            return typeof( Button );
        if( go.GetComponent<Image>() != null )
            return typeof( Image );
        if( go.GetComponent<Text>() != null )
            return typeof( Text );
        return typeof( Transform );
    }

    public static string GetGameObjectPath(GameObject obj)
    {
        int count = 0;
        int rootIndex = 0;
        List<string> list = new List<string>();
        list.Add( obj.name );

        while( obj.transform.parent != null )
        {
            obj = obj.transform.parent.gameObject;
            list.Add(obj.name);
            if( rootIndex == 0 && obj.name.Contains( GetObjectPath.FILTER_STRING_PREMIX ) )  //向上遍历到第一个"UI_XX"
            {
                rootIndex = count;
            }
            count++;
        }

        string strResult = "";
        for( int i = rootIndex; i >= 0; i-- )
        {
            strResult += list[ i ];
            if( i > 0 )
            {
                strResult += "|";
            }
        }
        return strResult;
    }

    //复制到剪切板
    private static TextEditor textEditor;
    public static void SaveToClipBoard(string content)
    {
        if( GetObjectPath.textEditor == null )
        {
            GetObjectPath.textEditor = new TextEditor();
        }
        //textEditor.content = new GUIContent( content );
        textEditor.text = content;
        textEditor.OnFocus();
        textEditor.Copy();
    }
}
