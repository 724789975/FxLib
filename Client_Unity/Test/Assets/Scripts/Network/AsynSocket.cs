///<Remark>PanYangzhong，创建日期:2016.10.1</ Remark >
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace Net
{

    public class AsynSocket
    {
        private Action<byte[], int> _receiveCallback;
        private Action<ConnectResult> _connectCallback;
        private Socket _socket;

        /// <summary>Position of the bom header in the rxBuffer</summary>
        protected int rxHeaderIndex = -1;
        /// <summary>Expected length of the message from the bom header</summary>
        protected int rxBodyLen = -1;
        /// <summary>Store incoming bytes to be processed</summary>
        protected byte[] byteBuffer = new byte[4192];
        /// <summary>Buffer of received data</summary>
        protected MemoryStream rxBuffer = new MemoryStream();

        public void AsynConnect(string ip, int port)
        {
            Disconnect();
            _receiveCallback = null;
            String newServerIp = "";
            AddressFamily newAddressFamily = AddressFamily.InterNetwork;
            getIPType(ip, port, out newServerIp, out newAddressFamily);
            IPEndPoint ipe = new IPEndPoint(IPAddress.Parse(newServerIp), port);
            _socket = new Socket(newAddressFamily, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                _socket.BeginConnect(ipe, result =>
                {
                    SampleDebuger.Log(result.ToString());
                    ConnectResult errCode = ConnectResult.Failed;
                    if (_socket.Connected)
                    {
                        AsynReceive();
                        errCode = ConnectResult.Success;
                    }

                    Loom.QueueOnMainThread(() =>
                    {
                        _connectCallback(errCode);
                    });
                }, null);
            }
            catch (Exception e)
            {
                SampleDebuger.LogError(e.Message);
            }
        }

        public bool IsConnected()
        {
            if (_socket != null && _socket.Connected)
            {
                return true;
            }
            return false;
        }

        public void Disconnect()
        {
            if (_socket != null && _socket.Connected)
            {
//                _socket.BeginDisconnect(true, ret =>
//                {
//                    _socket.EndDisconnect(ret);
//                }, null);
				_socket.Disconnect(false);
				_socket = null;
            }
        }

        public void SetReceiveFunc(Action<byte[], int> func)
        {
            _receiveCallback = func;
        }

        public void SetConnectFunc(Action<ConnectResult> func)
        {
            _connectCallback = func;
        }

        public void AsynSend(byte[] data)
        {
            if (!IsConnected())
            {
                //SampleDebuger.LogError("Socket has been disconnected from server");
                EventDispatcher.Instance.Emit("Network_Disconnected");
                return;
            }

            try
            {
                _socket.BeginSend(data, 0, data.Length, SocketFlags.None, result =>
                {
                    int length = _socket.EndSend(result);

                }, null);

            }
            catch (Exception e)
            {
                //SampleDebuger.LogError(e.Message);
                Loom.QueueOnMainThread(() =>
                {
                    EventDispatcher.Instance.Emit("Network_Disconnected");
                });
            }
        }

        public void AsynReceive()
        {
            //try
            //{
                _socket.BeginReceive(this.byteBuffer, 0, this.byteBuffer.Length, SocketFlags.None, result =>
                {
                    int size = _socket.EndReceive(result);
					if (size < 0)
					{
						SampleDebuger.LogError("length : " + size);
						return;
					}
					if (size == 0)
					{
						Loom.QueueOnMainThread(() =>
						{
							EventDispatcher.Instance.Emit("KickedByServer");
						});
						return;
					}
					lock (this.rxBuffer)
                    {
                        // put at the end for safe writing
                        this.rxBuffer.Position = this.rxBuffer.Length;
                        this.rxBuffer.Write(this.byteBuffer, 0, size);

                        bool more = false;
                        do
                        {
                            // search for header if not found yet
                            if (this.rxHeaderIndex < 0)
                            {
                                this.rxBuffer.Position = 0; // rewind to search
                                this.rxHeaderIndex = 0;
                            }

                            // have the header
                            if (this.rxHeaderIndex > -1)
                            {
                                // read the body length from header
                                if (this.rxBodyLen < 0 && this.rxBuffer.Length - this.rxHeaderIndex >= 4)
                                {
                                    this.rxBuffer.Position = this.rxHeaderIndex; // start reading after bomBytes
                                    this.rxBuffer.Read(this.byteBuffer, 0, 4); // read message length
                                    this.rxBodyLen = BitConverter.ToUInt16(this.byteBuffer, 0);
                                }

                                // we have the message
                                if (this.rxBodyLen > -1 && (this.rxBuffer.Length - this.rxHeaderIndex - 4) >= this.rxBodyLen)
                                {
                                    try
                                    {
                                        this.rxBuffer.Position = this.rxHeaderIndex + 4;
                                        byte[] data = new byte[this.rxBodyLen];
                                        this.rxBuffer.Read(data, 0, data.Length);
                                        _receiveCallback(data, data.Length);
                                    }
                                    catch (Exception ex)
                                    {
                                        SampleDebuger.LogError(ex.Message);
                                        Disconnect();
                                    }

                                    if (this.rxBuffer.Position == this.rxBuffer.Length)
                                    {
                                        // no bytes left
                                        // just resize buffer
                                        this.rxBuffer.SetLength(0);
                                        this.rxBuffer.Capacity = this.byteBuffer.Length;
                                        more = false;
                                    }
                                    else
                                    {
                                        // leftover bytes after current message
                                        // copy these bytes to the beginning of the rxBuffer
                                        this.CopyBack();
                                        more = true;
                                    }

                                    // reset header info
                                    this.rxHeaderIndex = -1;
                                    this.rxBodyLen = -1;
                                }
                                else if (this.rxHeaderIndex > 0)
                                {
                                    // remove bytes from before the header
                                    this.rxBuffer.Position = this.rxHeaderIndex;
                                    this.CopyBack();
                                    this.rxHeaderIndex = 0;
                                    more = false;
                                }
                                else
                                    more = false;
                            }
                        } while (more);
                    }
                    AsynReceive();
                }, null);
            //}
            //catch (Exception e)
            //{
             //   SampleDebuger.LogError(e.Message);
          //  }
        }

        /// <summary>Find first position the specified byte within the stream, or -1 if not found</summary>
        /// <param name="ms"></param>
        /// <param name="find"></param>
        /// <returns></returns>
        private int IndexOfByteInStream(MemoryStream ms, byte find)
        {
            int b;
            do
            {
                b = ms.ReadByte();
            } while (b > -1 && b != find);

            if (b == -1)
                return -1;
            else
                return (int)ms.Position - 1; // position is +1 byte after the byte we want
        }

        /// <summary>Find first position the specified bytes within the stream, or -1 if not found</summary>
        /// <param name="ms"></param>
        /// <param name="find"></param>
        /// <returns></returns>
        private int IndexOfBytesInStream(MemoryStream ms, byte[] find)
        {
            int index;
            do
            {
                index = this.IndexOfByteInStream(ms, find[0]);

                if (index > -1)
                {
                    bool found = true;
                    for (int i = 1; i < find.Length; i++)
                    {
                        if (find[i] != ms.ReadByte())
                        {
                            found = false;
                            ms.Position = index + 1;
                            break;
                        }
                    }
                    if (found)
                        return index;
                }
            } while (index > -1);
            return -1;
        }

        /// <summary>
        /// Copies the stuff after the current position, back to the start of the stream,
        /// resizes the stream to only include the new content, and
        /// limits the capacity to length + another buffer.
        /// </summary>
        private void CopyBack()
        {
            int count;
            long readPos = this.rxBuffer.Position;
            long writePos = 0;
            do
            {
                count = this.rxBuffer.Read(this.byteBuffer, 0, this.byteBuffer.Length);
                readPos = this.rxBuffer.Position;
                this.rxBuffer.Position = writePos;
                this.rxBuffer.Write(this.byteBuffer, 0, count);
                writePos = this.rxBuffer.Position;
                this.rxBuffer.Position = readPos;
            }
            while (count > 0);
            this.rxBuffer.SetLength(writePos);
            this.rxBuffer.Capacity = (int)this.rxBuffer.Length + this.byteBuffer.Length;
        }


#if UNITY_IPHONE && !UNITY_EDITOR
    [DllImport("__Internal")]
    private static extern string getIPv6(string mHost, string mPort);
#endif

        public string GetIPv6(string mHost, string mPort)
        {
#if UNITY_IPHONE && !UNITY_EDITOR
		string mIPv6 = getIPv6(mHost, mPort);
		return mIPv6;
#else
        return mHost + "&&ipv4";
#endif
        }


        void getIPType(String serverIp, int serverPorts, out String newServerIp, out AddressFamily mIPType)
        {
            mIPType = AddressFamily.InterNetwork;
            newServerIp = serverIp;
            try
            {
                string mIPv6 = GetIPv6(serverIp, serverPorts.ToString());
                if (!string.IsNullOrEmpty(mIPv6))
                {
                    string[] m_StrTemp = System.Text.RegularExpressions.Regex.Split(mIPv6, "&&");
                    if (m_StrTemp != null && m_StrTemp.Length >= 2)
                    {
                        string IPType = m_StrTemp[1];
                        if (IPType == "ipv6")
                        {
                            newServerIp = m_StrTemp[0];
                            mIPType = AddressFamily.InterNetworkV6;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                SampleDebuger.LogError("GetIPv6 error:" + e);
            }
        }
    }

}