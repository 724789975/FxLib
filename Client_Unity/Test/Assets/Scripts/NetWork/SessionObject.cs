﻿using System;
using System.Text;
using System.Collections.Generic;
using UnityEngine;
using FxNet;

public class SessionObject : MonoBehaviour
{
	public enum SessionType
	{
		SessionType_TCP,
		SessionType_WebSocket,
	}
	// Use this for initialization
	void Start ()
	{
        DontDestroyOnLoad(this);
	}
	
	// Update is called once per frame
	void Update ()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		if (Time.time - m_fLastUpdate < 0.05f)
		{
			return;
		}
		m_fLastUpdate = Time.time;
		if(m_pClientSocket != null)
		{
			m_pClientSocket.Update();
		}
#endif
	}

	public bool InitSession(SessionType eType, string szIp, UInt16 wPort)
	{
		SampleDebuger.Log("type : " + eType.ToString() + ", ip : " + szIp + ", port : " + wPort);
		m_szIP = szIp;
		m_wPort = wPort;
		m_eSessionType = eType;

		switch (m_eSessionType)
		{
			case SessionType.SessionType_TCP:
				{
					m_pSession = new BinarySession(this);
				}
				break;
			case SessionType.SessionType_WebSocket:
				{
					m_pSession = new WebSocketSession(this);
				}
				break;
			default:
				{
					return false;
					//throw new ArgumentException("error session type : " + m_eSessionType.ToString());
				}
				break;
		}

		m_pSession.Init(m_szIP, m_wPort);
		m_pClientSocket = m_pSession.Reconnect();
		return true;
	}

	public void OnClose()
	{
		foreach (var item in m_pfOnClose)
		{
			item();
		}
        m_pSession = null;
		//m_pfOnClose();
		//Debug.LogError("session closed");
	}

	public bool OnSessionDestroy()
	{
		SampleDebuger.LogError("session obj has destroy!!!!");
        Destroy(this);
		return false;
	}

	public void OnError(uint dwErrorNo)
	{
		foreach (var item in m_pfOnError)
		{
			item(dwErrorNo);
		}
		//m_pfOnError(dwErrorNo);
		//Debug.LogError("session error : " + dwErrorNo.ToString());
	}

	public bool Send(byte[] pBuf, uint dwLen)
	{
		return m_pSession.Send(pBuf, dwLen);
	}

	public void OnConnect()
	{
		foreach (var item in m_pfOnConnect)
		{
			item();
		}
		//m_pfOnConnect();

	}

	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		NetStream pStream = new NetStream(pBuf, dwLen);
		string szProtoName = "";
		pStream.ReadString(ref szProtoName);
		byte[] pProto = new byte[pStream.GetLeftLen()];
		pStream.ReadData(ref pProto, pStream.GetLeftLen());

		if (!m_mapCallBack.ContainsKey(szProtoName))
		{
			SampleDebuger.Log("can't find proto name " + szProtoName);
			return;
		}
		SampleDebuger.LogGreen(szProtoName);
		m_mapCallBack[szProtoName](pProto);

		//foreach (var item in m_pfOnRecv)
		//{
		//	item(pBuf, dwLen);
		//}
		//m_pfOnRecv(pBuf, dwLen);
	}

	public void RegistMessage(string szProtoName, Action<byte[]> pfCallBack)
	{
		if (m_mapCallBack.ContainsKey(szProtoName))
		{
			SampleDebuger.LogError("already registed proto " + szProtoName);
			return;
		}
		m_mapCallBack[szProtoName] = pfCallBack;
	}

    public void UnRegistMessage(string szProtoName)
    {
        m_mapCallBack.Remove(szProtoName);
    }

    FxNet.IFxClientSocket GetClientSocket() { return m_pClientSocket; }

	public FxNet.ISession m_pSession = null;
	public SessionType m_eSessionType = SessionType.SessionType_TCP;
	public string m_szIP;
	public UInt16 m_wPort = 0;
	FxNet.IFxClientSocket m_pClientSocket;

	public HashSet<Action> m_pfOnConnect = new HashSet<Action>();
	public HashSet<Action> m_pfOnClose = new HashSet<Action>();
	public HashSet<Action<uint>> m_pfOnError = new HashSet<Action<uint>>();

	public Dictionary<string, Action<byte[]>> m_mapCallBack = new Dictionary<string, Action<byte[]>>();

#if UNITY_WEBGL && !UNITY_EDITOR
	float m_fLastUpdate = 0.0f;
#endif
}
