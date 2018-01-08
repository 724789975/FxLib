using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace FxNet
{
    public abstract class FxClientSocket
	{
		public class StateObject
		{
			public Socket workSocket = null;
			public const int BUFFER_SIZE = 1024;
			public byte[] buffer = new byte[BUFFER_SIZE];
			public StringBuilder sb = new StringBuilder();
		}
		private Socket m_hSocket;
		private bool m_bReconnect;
		private string m_szIp;
		private int m_nPort;

		public bool Init()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// 这个是在线程中执行的 要注意
		/// </summary>
		public void Update()
		{
			throw new System.NotImplementedException();
		}

		public bool IsConnected()
		{
			throw new System.NotImplementedException();
		}

		public void ProcEvent(SNetEvent pEvent)
		{
			throw new System.NotImplementedException();
		}

		public void Connect(string szIp, int nPort, bool bReconnect)
		{
			m_szIp = szIp;
			m_nPort = nPort;
			m_bReconnect = bReconnect;
			m_hSocket.BeginConnect(m_szIp, m_nPort, new AsyncCallback(ConnectCallback), this);
		}

		private void ConnectCallback(IAsyncResult ar)
		{
			try
			{
				FxClientSocket pClientSocket = (FxClientSocket)ar.AsyncState;
				pClientSocket.m_hSocket.EndConnect(ar);
			}
			catch (SocketException ex)
			{ }
		}

		private void Send(byte[] byteData)
		{
			// Begin sending the data to the remote device.     
			m_hSocket.BeginSend(byteData, 0, byteData.Length, 0, new AsyncCallback(SendCallback), this);
		}
		private void SendCallback(IAsyncResult ar)
		{
			try
			{
				// Retrieve the socket from the state object.     
				FxClientSocket pClientSocket = (FxClientSocket)ar.AsyncState;
				// Complete sending the data to the remote device.     
				int bytesSent = pClientSocket.m_hSocket.EndSend(ar);
			}
			catch (Exception e)
			{
			}
		}

		private void Receive()
		{
			try
			{
				// Create the state object.     
				StateObject state = new StateObject();
				state.workSocket = m_hSocket;
				// Begin receiving the data from the remote device.     
				m_hSocket.BeginReceive(state.buffer, 0, StateObject.BUFFER_SIZE, 0, new AsyncCallback(ReceiveCallback), state);
			}
			catch (Exception e)
			{
			}
		}
		private static void ReceiveCallback(IAsyncResult ar)
		{
			try
			{
				// Retrieve the state object and the client socket     
				// from the asynchronous state object.     
				StateObject state = (StateObject)ar.AsyncState;
				Socket client = state.workSocket;
				// Read data from the remote device.     
				int bytesRead = client.EndReceive(ar);
				if (bytesRead > 0)
				{
					// There might be more data, so store the data received so far.     
					state.sb.Append(Encoding.ASCII.GetString(state.buffer, 0, bytesRead));
					// Get the rest of the data.     
					client.BeginReceive(state.buffer, 0, StateObject.BUFFER_SIZE, 0, new AsyncCallback(ReceiveCallback), state);
				}
				else
				{
					// All the data has arrived; put it in response.     
					if (state.sb.Length > 1)
					{
						string response = state.sb.ToString();
					}
					client.Close();
				}
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
			}
		}
	}
}