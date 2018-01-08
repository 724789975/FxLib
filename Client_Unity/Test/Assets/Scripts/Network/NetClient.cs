///<Remark>PanYangzhong，创建日期:2016.11.17</ Remark >
using System;
using Protocol;
using System.Collections.Generic;
using UnityEngine;
using LuaInterface;

namespace Net
{
    public enum ConnectResult
    {
        None,
        Success,
        Failed,
        NoNetwork
    }

    public class ReceiveQueueItem
    {
        private Action<ReceiveMessage> _fn;
        private ReceiveMessage _msg;
        public ReceiveQueueItem(Action<ReceiveMessage> fn, ReceiveMessage msg)
        {
            _fn = fn;
            _msg = msg;
        }

        public void Call()
        {
            _fn(_msg);
        }
    }

    public class NetClient : UEventEmitter
    {
        private static NetClient _client;

        private AsynSocket _gameSocket; //主套接字，连接Login, Game 服务器

        private object _receiveLock = new object(); //接收数据锁

        ///回调函数表
        private Dictionary<uint, Action<ReceiveMessage>> _callbackDic = new Dictionary<uint, Action<ReceiveMessage>>();

        //回调函数表(回调后移除)
        private Dictionary<uint, Action<ReceiveMessage>> _onceCallbackDic = new Dictionary<uint, Action<ReceiveMessage>>();

        private List<ReceiveQueueItem> _receiveQueue = new List<ReceiveQueueItem>();

        private Queue<SendMessage> _resendQueue = new Queue<SendMessage>();

        public static NetClient Instance
        {
            get
            {
                if (_client == null)
                {
                    GameObject obj = new GameObject();
                    obj.name = "NetClient";
                    obj.AddComponent<DestoryWhenReboot>();
                    _client = obj.AddComponent<NetClient>();
                    _client.ListenHeartBeat();
                }
                return _client;
            }
        }

        /// <summary>
        /// 注册回调
        /// </summary>
        /// <param name="msgNum">消息ID</param>
        /// <param name="cb">回调</param>
        public void On(MsgEnum msgNum, Action<ReceiveMessage> cb)
        {
            uint id = (uint)msgNum;
            lock (_receiveLock)
            {
                if (_callbackDic.ContainsKey(id))
                {
                    SampleDebuger.LogWarning("Msg " + id + " has been registered!");
                }
                else
                {
                    _callbackDic.Add(id, cb);
                }
            }
        }

        /// <summary>
        /// 注销回调
        /// </summary>
        /// <param name="msgNum">消息ID</param>
        public void Off(MsgEnum msgNum)
        {
            uint id = (uint)msgNum;
            lock (_receiveLock)
            {
                if (!_callbackDic.ContainsKey(id))
                {
                    SampleDebuger.LogWarning("Can't find msg: " + id);
                }
                else
                {
                    _callbackDic.Remove(id);
                }
            }
        }

        /// <summary>
        /// 注册单次回调
        /// </summary>
        /// <param name="msgNum">消息ID</param>
        /// <param name="cb">回调函数</param>
        public void Once(MsgEnum msgNum, Action<ReceiveMessage> cb)
        {
            uint id = (uint)msgNum;
            lock (_receiveLock)
            {
                if (_callbackDic.ContainsKey(id) || _onceCallbackDic.ContainsKey(id))
                {
                    SampleDebuger.LogWarning("Msg " + id + " has been registered!");
                }
                else
                {
                    _onceCallbackDic.Add(id, cb);
                }
            }
        }

        /// <summary>
        /// 接收&发送心跳包
        /// </summary>
        void ListenHeartBeat()
        {
            On(MsgEnum.LOGIN_ACK_CLIENT_HEART_BEAT, _loginServerHB);
            On(MsgEnum.GAME_ACK_CLIENT_HEART_BEAT,  _gameServerHB);
            EventDispatcher.Instance.On("MainPlayer_DataLoaded", Resend);
        }

        void _loginServerHB(ReceiveMessage res)
        {
            NullStruct msg = new NullStruct();
            Send(MsgEnum.CLIENT_AND_LOGIN_HEART_BEAT, msg);
            //SampleDebuger.Log("HB from Login Server!");
        }

        void _gameServerHB(ReceiveMessage res)
        {
            NullStruct msg = new NullStruct();
            Send(MsgEnum.CLIENT_AND_GAME_HEART_BEAT, msg);
            //SampleDebuger.Log("HB from Game Server!");
        }

        ///接收二进制数据
        private void OnReceive(byte[] data, int length)
        {
            ReceiveMessage msg = new ReceiveMessage(data, length);
#if UNITY_EDITOR
            System.Threading.Thread.Sleep(1);
#endif
            OnMessage(msg);
        }

        /// 接收到消息
        private void OnMessage(ReceiveMessage msg)
        {
            lock (_receiveLock)
            {
                msg.ReadHead();
                uint id = msg.GetID();

                if(Config.MsgDebugFlag)
                {
                    foreach (MsgEnum item in Enum.GetValues(typeof(MsgEnum)))
                    {
                        if (item == MsgEnum.CLIENT_AND_LOGIN_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.LOGIN_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.CLIENT_AND_GAME_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.GAME_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.CLIENT_AND_RELATION_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.RELATION_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.LOGIN_ACK_CLIENT_NET_PING)
                            continue;

                        if ((uint)item == id)
                            SampleDebuger.LogColor(item, "green");
                    }
                }
                Action<ReceiveMessage> cb;
                _callbackDic.TryGetValue(id, out cb);
                if (cb == null)
                {
                    _onceCallbackDic.TryGetValue(id, out cb);
                    _onceCallbackDic.Remove(id);
                }

                if (cb != null)
                {
                    _addCMessage(cb, msg);
                }
                else
                {
                    _addLuaMessage(msg);
                }
            }
        }

        void _addCMessage(Action<ReceiveMessage> cb, ReceiveMessage msg)
        {
            _receiveQueue.Add(new ReceiveQueueItem(cb, msg));
        }

        void _addLuaMessage(ReceiveMessage msg)
        {
            _receiveQueue.Add(new ReceiveQueueItem(_dispatchLuaMessage, msg));
        }

        void _dispatchLuaMessage(ReceiveMessage msg)
        {
            uint id = msg.GetID();
            LuaManager.Instance.CallFunction("NetworkDispatch", id, msg._reader);
        }

        void Update()
        {
            lock (_receiveLock)
            {
                if (_receiveQueue.Count > 0)
                {

                    for (int i = 0; i < _receiveQueue.Count; i++)
                    {
                        try
                        {
                            ReceiveQueueItem item = _receiveQueue[i];
                            item.Call();
                        }
                        catch (Exception e)
                        {
                            SampleDebuger.LogError(e.ToString());
                        }
                    }
                    _receiveQueue.Clear();
                }
            }
        }

        #region 主服务器方法
        public void Connect(string ip, int port, Action<ConnectResult> cb = null)
        {
            if (Application.internetReachability == NetworkReachability.NotReachable)
            {
                cb(ConnectResult.NoNetwork);
            }
            else
            {
                if (_gameSocket == null)
                {
                    _gameSocket = new AsynSocket();
                }
                _gameSocket.AsynConnect(ip, port);
                _gameSocket.SetReceiveFunc(OnReceive);
                _gameSocket.SetConnectFunc(cb);
            }
        }

        public void Disconnect()
        {
            if (_gameSocket != null)
            {
                _gameSocket.Disconnect();
                Emit("Socket_Disconnected");
            }

        }

        public bool IsConnected()
        {
            return _gameSocket != null && _gameSocket.IsConnected();
        }

        /// <summary>
        /// 发送消息
        /// </summary>
        /// <param name="id"></param>
        /// <param name="data"></param>
        public bool Send(uint id, CBaseMsg data, bool resend = false)
        {

            if (!IsConnected())
            {
                if (resend)
                {
                    SendMessage msg = new SendMessage(id);
                    msg.Write(data);
                    _resendQueue.Enqueue(msg);
                }
                return false;
            }
            else
            {
                SendMessage msg = new SendMessage(id);
                msg.Write(data);
                _gameSocket.AsynSend(msg.GetBuffer());

                if (Config.MsgDebugFlag)
                {
                    foreach (MsgEnum item in Enum.GetValues(typeof(MsgEnum)))
                    {
                        if (item == MsgEnum.CLIENT_AND_LOGIN_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.LOGIN_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.CLIENT_AND_GAME_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.GAME_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.CLIENT_AND_RELATION_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.RELATION_ACK_CLIENT_HEART_BEAT)
                            continue;
                        if (item == MsgEnum.CLIENT_REQ_LOGIN_NET_PING)
                            continue;

                        if ((uint)item == id)
                            SampleDebuger.LogColor(item, "yellow");
                    }
                }
                return true;
            }
        }

        /// <summary>
        /// 发送消息
        /// </summary>
        /// <param name="id"></param>
        /// <param name="data"></param>
        public void Send(MsgEnum id, CBaseMsg data)
        {
            Send((uint)id, data);
        }

        public void SafeSend(uint id, CBaseMsg data)
        {
            Send(id, data, true);
        }

        public void Resend(params object[] args)
        {
            while (_resendQueue.Count > 0) 
            {
                SampleDebuger.Log("+++++ ReSending Message+++++");
                SendMessage msg = _resendQueue.Dequeue();
                _gameSocket.AsynSend(msg.GetBuffer());
            }

        }


        /// <summary>
        /// 请求数据
        /// </summary>
        /// <param name="reqEnum">请求ID</param>
        /// <param name="data">请求数据</param>
        /// <param name="resEnum">返回ID</param>
        /// <param name="cb">返回数据</param>
        public void Request(MsgEnum reqEnum, CBaseMsg data, MsgEnum resEnum, Action<ReceiveMessage> cb)
        {
            Once(resEnum, cb);
            Send(reqEnum, data);
        }
        #endregion

        void OnDestory()
        {
            Disconnect();
        }
    }

}

