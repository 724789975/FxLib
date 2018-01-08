//Pan Yangzhong 创建日期：2017/6/30
//监测网络状况
using UnityEngine;
using Net;
using Protocol;

public class NetworkMonitor : UEventEmitter
{
    int _checkCount = 0;
    public int IntervalSec = 1;
    public int CheckSum = 3;
    void Start()
    {
        On("MainPlayer_DataLoaded", _startLoginListen);
        On("Socket_Disconnected", _netwokDisconnected);
        NetClient.Instance.On(MsgEnum.LOGIN_ACK_CLIENT_NET_PING,    _pongFromLoginServer);
    }

    void _startLoginListen(params object[] args)
    {
        InvokeRepeating("_checkLoginNetwork", 0, IntervalSec);
        _checkCount = 0;
    }

    void _netwokDisconnected(params object[] args)
    {
        CancelInvoke("_checkLoginNetwork");
        _checkCount = 0;
    }

    void _pongFromLoginServer(ReceiveMessage res)
    {
        _checkCount = 0;
       // SampleDebuger.LogGreen("receive checksum: " + _checkCount);
    }

    void _checkLoginNetwork()
    {
        if (Application.internetReachability == NetworkReachability.NotReachable || _checkCount > CheckSum)
        {
            //Debug.LogError("CheckSum: " + _checkCount);
            CancelInvoke("_checkLoginNetwork");
            _checkCount = 0;
            Emit("ResetLoginManager");
        }
        else
        {
            _checkCount++;
            //SampleDebuger.LogGreen("send checksum: " + _checkCount);
            NullStruct msg = new NullStruct();
            NetClient.Instance.Send(MsgEnum.CLIENT_REQ_LOGIN_NET_PING, msg);
        }
    }

    //void OnDestory()
    //{
    //    CancelInvoke("_checkLoginNetwork");
    //}
}
