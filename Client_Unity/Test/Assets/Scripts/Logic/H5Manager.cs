using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class H5Manager : SingletonObject<H5Manager>
{
	void Awake()
	{
		CreateInstance(this);
	}
	// Use this for initialization
	void Start () { }
	
	// Update is called once per frame
	void Update () { }

	public void Init(ushort wPlayerPort, ushort wServerPort, ushort wSlaveServerPort)
	{
		m_wPlayerPort = wPlayerPort;
		m_wServerPort = wServerPort;
		m_wSlaveServerPort = wSlaveServerPort;

		m_pServerSession.Init(SessionObject.SessionType.SessionType_WebSocket, "127.0.0.1", m_wServerPort);
		//m_pServerSession = this.gameObject.AddComponent<SessionObject>();
		//m_pServerSession.m_eSessionType = SessionObject.SessionType.SessionType_WebSocket;
		//m_pServerSession.m_szIP = "127.0.0.1";
		//m_pServerSession.m_wPort = m_wServerPort;
	}

	public void Init()
	{
		m_pServerSession.Init(SessionObject.SessionType.SessionType_WebSocket, "127.0.0.1", m_wServerPort);
	}

	public SessionObject GetServerSession() { return m_pServerSession; }

	public ushort m_wPlayerPort = 0;
	public ushort m_wServerPort = 0;
	public ushort m_wSlaveServerPort = 0;

	public SessionObject m_pServerSession;
}
