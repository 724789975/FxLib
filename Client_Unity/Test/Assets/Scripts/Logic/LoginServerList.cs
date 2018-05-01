using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
public class ServerListInfo
{
	public List<ServerInfo> server_infos = new List<ServerInfo>();
}


public class LoginServerList : MonoBehaviour
{
	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SetServerListInfo(ServerListInfo pServerListInfo)
	{
		for (int i = 0; i < pServerListInfo.server_infos.Count; i++)
		{
			GameObject obj = (GameObject)Instantiate(m_pObj, m_pContentTransform);
			LoginServerInfo oInfo = obj.GetComponent<LoginServerInfo>();

			//Vector3 v3 = new Vector3(0, m_pScrollRect.content.rect.height / 2 - (i + 0.5f) * oInfo.m_pButton.GetComponent<RectTransform>().rect.height, 0);
			////obj.GetComponent<RectTransform>().localPosition = v3;
			////obj.GetComponent<RectTransform>().position = v3;
			//obj.transform.position = v3;
			oInfo.SetServerInfo(pServerListInfo.server_infos[i]);
		}

    }

	public Object m_pObj;
	public Transform m_pContentTransform;
}
