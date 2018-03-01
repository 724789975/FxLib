﻿using System;
using System.Text;
using System.Collections.Generic;
using UnityEngine;

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
	}
	
	// Update is called once per frame
	void Update ()
	{
#if UNITY_WEBGL && !UNITY_EDITOR
		if(m_pClientSocket != null)
		{
			m_pClientSocket.Update();
		}
#else
#endif
	}

	public bool InitSession(SessionType eType, string szIp, UInt16 wPort)
	{
		H5Helper.H5LogStr("type : " + eType.ToString() + ", ip : " + szIp + ", port : " + wPort);
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
		//m_pfOnClose();
		//Debug.LogError("session closed");
	}

	public bool OnDestroy()
	{
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

		string szData = String.Format("{0}, {1}, {2}, {3}, {4}",
			"sessionobject.cs", 83, "SessionObject::OnConnect",
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		Send(pData, (UInt32)pData.Length);
	}

	int dw1 = 0;
	public void OnRecv(byte[] pBuf, uint dwLen)
	{
		foreach (var item in m_pfOnRecv)
		{
			item(pBuf, dwLen);
		}
		//m_pfOnRecv(pBuf, dwLen);

		string szData = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());
		byte[] pData = Encoding.UTF8.GetBytes(szData);
		Send(pData, (UInt32)pData.Length);
	}

	FxNet.IFxClientSocket GetClientSocket() { return m_pClientSocket; }

	public FxNet.ISession m_pSession;
	public SessionType m_eSessionType = SessionType.SessionType_TCP;
	public string m_szIP;
	public UInt16 m_wPort = 0;
	FxNet.IFxClientSocket m_pClientSocket;

	public delegate void PFun();
	public delegate void PFun1(uint p1);
	public delegate void PFun2(byte[] p1, uint p2);
	//public Action m_pfOnConnect;
	//public Action<byte[], uint> m_pfOnRecv;
	//public Action m_pfOnClose;
	//public Action<uint> m_pfOnError;
	public HashSet<PFun> m_pfOnConnect = new HashSet<PFun>();
	public HashSet<PFun2> m_pfOnRecv = new HashSet<PFun2>();
	public HashSet<PFun> m_pfOnClose = new HashSet<PFun>();
	public HashSet<PFun1> m_pfOnError = new HashSet<PFun1>();
}
