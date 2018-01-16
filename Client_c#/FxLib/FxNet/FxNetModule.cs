using System;
using System.Collections.Generic;

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
		public IFxClientSocket pSock;
		public SNetEvent pEvent;
	}

	public class FxNetModule : TSingleton<FxNetModule>
	{
		public bool Init()
		{
			m_pEventQueue = new List<SSockNetEvent>();
			m_pLock = new object();
			return true;
		}
		public bool Run(UInt32 dwCount = 0xffffffff)
		{
			List<SSockNetEvent> lProcEvent = null;
			lock (m_pLock)
			{
				lProcEvent = m_pEventQueue;
				m_pEventQueue = new List<SSockNetEvent>();
			}

			foreach (SSockNetEvent poEvent in lProcEvent)
			{
				poEvent.pSock.ProcEvent(poEvent.pEvent);
			}
			lProcEvent.Clear();
			return false;
		}
		public void Release() { }

		public bool Uninit() { return false; }

		public void PushNetEvent(IFxClientSocket poSock, SNetEvent pEvent)
		{
			lock(m_pLock)
			{
				SSockNetEvent pSocketEvent = new SSockNetEvent();
				pSocketEvent.pSock = poSock;
				pSocketEvent.pEvent = pEvent;
				m_pEventQueue.Add(pSocketEvent);
			}
		}

		/// <summary>
		/// 事件队列 这个会放到多线程中 不过push_back与pop_front都只各有一个线程 所以不用加锁
		/// </summary>
		private List<SSockNetEvent> m_pEventQueue;
		object m_pLock;
	}
}