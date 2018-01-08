using UnityEngine;
using System.Collections;
using System;
using Net;
using Protocol;

public class GameTime : MonoBehaviour {
    static GameTime sInstance = null;

    int _diffSec = 0;
    // Use this for initialization
	void Awake () {
        sInstance = this;
        gameObject.AddComponent<DestoryWhenReboot>();
    }
	
    public static GameTime Instance
    {
        get
        {
            return sInstance;
        }
    }

    public void UpdateServerTime()
    {
        NullStruct msg = new NullStruct();
        float startTime = Time.time;
        NetClient.Instance.Request(Protocol.MsgEnum.CLIENT_REQUEST_TIME, msg, Protocol.MsgEnum.LOGIN_ACK_CLIENT_TIME, (res) =>
        {
            L2CAckRequestTime data = res.Read<L2CAckRequestTime>();
            float endTime = Time.time;
            SetServerTime((int)data.dwTimeStamp, (int)(startTime-endTime));
        });
    }

    public void SetServerTime(int sTime, int lactency)
    {
        TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1);
        _diffSec = sTime - (int)ts.TotalSeconds -  lactency/2;
        Debug.Log("SetServerTime diff " + _diffSec );
    }
    
    public int GetDiffSeconds()
    {
        return _diffSec;
    }
}
