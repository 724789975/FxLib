using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FxNet
{
	public enum ENetEvtType
	{
		NETEVT_INVALID = 0,
		NETEVT_ESTABLISH,
		NETEVT_ASSOCIATE,
		NETEVT_RECV,
		NETEVT_RECV_PACKAGE_ERROR,
		NETEVT_CONN_ERR,
		NETEVT_ERROR,
		NETEVT_TERMINATE,
		NETEVT_RELEASE,
	};

	public struct SNetEvent
	{
		public ENetEvtType eType;
		public UInt32 dwValue;
	};

	struct SSockNetEvent
	{
		public FxClientSocket pSock;
		public SNetEvent pEvent;
	}

	public class FxNetModule : TSingleton<FxNetModule>
	{

		public bool Init()
		{
			m_pEventQueue = new List<SSockNetEvent>();
			return true;
		}
		public bool Run(UInt32 dwCount = 0xffffffff)
		{
			dwCount = (UInt32)m_pEventQueue.Count;
			while (dwCount > 0)
			{
				m_pEventQueue[0].pSock.ProcEvent(m_pEventQueue[0].pEvent);
				m_pEventQueue.RemoveAt(0);
				--dwCount;
			}
			return false;
		}
		public void Release() { }

		public FxTcpClientSocket TcpConnect(string szIP, UInt16 wPort, bool bReconnect = false)
		{
			FxTcpClientSocket poSock = new FxTcpClientSocket();
			return poSock;
		}

		public FxUdpClientSocket UdpConnect(string szIP, UInt16 wPort, bool bReconnect = false) { return null; }

		public bool Uninit() { return false; }

		public void PushNetEvent(FxClientSocket poSock, SNetEvent pEvent)
		{
			SSockNetEvent pSocketEvent = new SSockNetEvent();
			pSocketEvent.pSock = poSock;
			pSocketEvent.pEvent = pEvent;
			m_pEventQueue.Add(pSocketEvent);
		}

		/// <summary>
		/// 事件队列 这个会放到多线程中 不过push_back与pop_front都只各有一个线程 所以不用加锁
		/// </summary>
		private List<SSockNetEvent> m_pEventQueue;
	}
}