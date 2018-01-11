using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace FxNet
{
	public class IoThread : TSingleton<IoThread>
	{
		void ThrdFunc()
		{
			while (!m_bStop)
			{
				foreach (IFxClientSocket poSock in m_setDelSockets)
				{
					m_setConnectSockets.Remove(poSock);
				}
				m_setDelSockets.Clear();
				foreach (IFxClientSocket poSock in m_setConnectSockets)
				{
					if (poSock.IsConnected())
					{
						poSock.Update();
					}
					else
					{
						poSock.Disconnect();
					}
				}
				lock(m_pLock)
				{
					foreach (IFxClientSocket poSock in m_setAddSockets)
					{
						poSock.Receive();
						m_setConnectSockets.Add(poSock);
					}
					m_setAddSockets.Clear();
				}
				Thread.Sleep(1);
			}
		}
		public bool Start()
		{
			m_poThrdHandler.Start();
			return true;
		}
		public void Stop() { m_bStop = true; }

		public bool Init()
		{
			m_pLock = new object();
			m_setConnectSockets = new HashSet<IFxClientSocket>();
			m_setAddSockets = new HashSet<IFxClientSocket>();
			m_setDelSockets = new HashSet<IFxClientSocket>();
			m_poThrdHandler = new Thread(new ThreadStart(this.ThrdFunc));
			m_bStop = false;
			return true;
		}
		public void Uninit() { }

		/// <summary>
		/// 主线程执行
		/// </summary>
		/// <param name="pSock"></param>
		public void AddConnectSocket(IFxClientSocket pSock)
		{
			lock(m_pLock)
			{
				m_setAddSockets.Add(pSock);
			}
		}
		/// <summary>
		/// 工作线程执行
		/// </summary>
		/// <param name="pSock"></param>
		public void DelConnectSocket(IFxClientSocket pSock)
		{
			lock(m_pLock)
			{
				m_setDelSockets.Add(pSock);
			}
		}

		object m_pLock;
		Thread m_poThrdHandler;

		bool m_bStop;

		//存放连接指针
		HashSet<IFxClientSocket> m_setConnectSockets;

		HashSet<IFxClientSocket> m_setAddSockets;
		HashSet<IFxClientSocket> m_setDelSockets;
	}
}