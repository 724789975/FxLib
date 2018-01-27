using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using WebSocketSharp;

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

			//for (int i = 0; i < 100; ++i)
			{
				WebSocketSession pSession = new WebSocketSession();
				pSession.Init("ws://127.0.0.1:20001/chat", 20001);
				pSession.Reconnect();
			}

			while (true)
			{
				FxNet.FxNetModule.Instance().Run();

				Thread.Sleep(100);
			}

			//WebSocket oSock = new WebSocket("asdf");
			//oSock.ConnectAsync();
			//oSock.ConnectAsync();

        }
    }
}
