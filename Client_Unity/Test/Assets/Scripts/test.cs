using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class test : MonoBehaviour
{

	// Use this for initialization
	void Start ()
	{
		FxNet.FxNetModule.CreateInstance();
		FxNet.FxNetModule.Instance().Init();
		FxNet.IoThread.CreateInstance();
		FxNet.IoThread.Instance().Init();
		FxNet.IoThread.Instance().Start();

		//for (int i = 0; i < 100; ++i)
		//{
		//	BinarySession pSession = new BinarySession(this);
		//	pSession.Init("127.0.0.1", 20001);
		//	pSession.Reconnect();
		//}

		WebSocketSession pSession = new WebSocketSession(this);
		pSession.Init("ws://127.0.0.1:20001/chat", 20001);
		pSession.Reconnect();
	}
	
	// Update is called once per frame
	void Update ()
	{
		FxNet.FxNetModule.Instance().Run();
	}

	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		string szData = Encoding.UTF8.GetString(pBuf);
		Debug.Log(szData);
	}
}
