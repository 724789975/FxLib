using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace test
{
    class Program
    {
		public void fun()
		{ }
        static void Main(string[] args)
        {
			FxNet.FxNetModule.CreateInstance();
			FxNet.FxNetModule.Instance().Init();
			FxNet.IoThread.CreateInstance();
			FxNet.IoThread.Instance().Init();
			FxNet.IoThread.Instance().Start();

			FxNet.FxTcpClientSocket tcp = new FxNet.FxTcpClientSocket();
			tcp.Init("127.0.0.1", 20001, true);
			tcp.Connect();

			while (true)
			{
				FxNet.FxNetModule.Instance().Run();
				var st = new System.Diagnostics.StackTrace();
				string szData = st.GetFrame(0).ToString();
				szData += "           ";
				szData += DateTime.Now.ToLocalTime().ToString();
				byte[] pData = Encoding.UTF8.GetBytes(szData);
				tcp.Send(pData, (UInt32)pData.Length);
				Thread.Sleep(100);
			}
        }
    }
}
