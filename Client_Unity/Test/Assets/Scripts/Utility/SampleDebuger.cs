using UnityEngine;
using Junfine.Debuger;

/// <summary>
/// Log 封装类
/// </summary>
public class SampleDebuger {

#if RELEASE_VER
	static private bool enableLog = true;
	static private bool enableLogAll = true;//启用所有log
#else
    static private bool enableLog = false;
    static private bool enableLogAll = true; //启用所有log
#endif
    static private bool enableLogError = true;
    static private bool enableLogWarning = true;

    static public void Log(object message) {
        if (enableLogAll && enableLog) {
            Debuger.Log(message.ToString(), "");
        }
    }
          
    static public void Log(object message, Object context) {
        if (enableLogAll && enableLog) {
            Debuger.Log(message.ToString(), context);
        }
    }

    static public void LogColor(object message, string color = "green") {
        if (enableLogAll && enableLog) {
            Debuger.Log(string.Format("<color={0}>{1}</color>", color, message.ToString()), "");
        }
    }

    static public void LogGreen(object message)
    {
        LogColor(message, "green");
    }

    static public void LogRed(object message)
    {
        LogColor(message, "red");
    }

    static public void LogBlue(object message)
    {
        LogColor(message, "blue");
    }

    static public void LogError(object message) 
    {
        if (enableLogAll && enableLog) {
            Debuger.LogError(message.ToString(), "");
        }
    }

    static public void LogError(object message, Object context) {
        if (enableLogAll && enableLogError) {
            Debuger.LogError(message.ToString(), context);
        }
    }

    static public void LogWarning(object message) {
        if (enableLogAll && enableLogError) {
            Debuger.LogWarning(message.ToString(), "");
        }
    }

    static public void LogWarning(object message, Object context) {
        if (enableLogAll && enableLogWarning) {
            Debuger.LogWarning(message.ToString(), context);
        }
    }
}