using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxNet
{
	public abstract class ISession
	{
		public abstract void OnConnect();

		public abstract void OnClose();

		public abstract void OnError(UInt32 dwErrorNo);

		public abstract void OnRecv(byte[] pBuf, UInt32 dwLen);

		public abstract void Release();

		public abstract bool Send(byte[] pBuf, UInt32 dwLen);

		public abstract void Close();

		public abstract IFxClientSocket Reconnect();

		public abstract bool IsConnected();

		public abstract void Init(string szIp, UInt16 wPort);

		public IFxDataHeader GetDataHeader() { return m_pDataHeader; }

		public abstract bool OnDestroy();

		protected IFxDataHeader m_pDataHeader;
		protected IFxClientSocket m_pSocket;

		protected string m_szIp;
		protected UInt16 m_wPort;
	}
}
