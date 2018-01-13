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
		public abstract bool IsConnecting();

		public abstract void Init();

		public IFxDataHeader GetDataHeader() { return m_pDataHeader; }

		public abstract bool OnDestroy();

		IFxDataHeader m_pDataHeader;
		IFxClientSocket m_pSocket;

		string m_szIp;
		UInt16 m_wPort;
	}
}
