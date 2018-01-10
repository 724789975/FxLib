using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace FxNet
{
	public class IoThread
	{
		void ThrdFunc()
		{
			while (!m_bStop)
			{
				foreach (IFxClientSocket poSock in m_setConnectSockets)
				{
					if (poSock.IsConnected())
					{
						poSock.Update();
					}
				}
				Thread.Sleep(1);
			}
		}
		public bool Start()
		{
			m_poThrdHandler.Start(this);
			return true;
		}
		public void Stop() { m_bStop = true; }

		public bool Init()
		{
			m_pLock = new object();
			m_setConnectSockets = new HashSet<IFxClientSocket>();
			m_poThrdHandler = new Thread(new ThreadStart(this.ThrdFunc));
			m_bStop = false;
			return true;
		}
		public void Uninit() { }

		/// <summary>
		/// 主线程执行
		/// </summary>
		/// <param name="pSock"></param>
		void AddConnectSocket(IFxClientSocket pSock)
		{
			lock(m_pLock)
			{
				m_setConnectSockets.Add(pSock);
			}
		}
		/// <summary>
		/// 主线程执行
		/// </summary>
		/// <param name="pSock"></param>
		void DelConnectSocket(IFxClientSocket pSock)
		{
			lock(m_pLock)
			{
				m_setConnectSockets.Remove(pSock);
			}
		}

		object m_pLock;
		Thread m_poThrdHandler;

		bool m_bStop;

		//存放连接指针
		HashSet<IFxClientSocket> m_setConnectSockets;
	}
}