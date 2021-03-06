﻿namespace Junfine.Debuger
{
    using System;
    using UnityEngine;

    public class Debuger
    {
        public static bool enableLog = true;

        public static void Log(string str, params object[] args)
        {
            if (enableLog)
            {
                str = string.Format(str, args);
                Debug.Log(str);
            }
        }

		public static void Log(string str)
		{
			if (enableLog)
			{
				Debug.Log(str);
			}
		}

		public static void LogWarning(string str)
		{
			if (enableLog)
			{
				Debug.LogWarning(str);
			}
		}

		public static void LogWarning(string str, params object[] args)
        {
            if (enableLog)
            {
                str = string.Format(str, args);
                Debug.LogWarning(str);
            }
		}
		public static void LogError(string str)
		{
			if (enableLog)
			{
				Debug.LogError(str);
			}
		}

		public static void LogError(string str, params object[] args)
		{
			if (enableLog)
			{
				str = string.Format(str, args);
				Debug.LogError(str);
			}
		}

	}
}

