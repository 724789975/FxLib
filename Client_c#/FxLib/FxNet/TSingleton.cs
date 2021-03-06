﻿
namespace FxNet
{
	public abstract class TSingleton<T> where T:new()
	{
		~TSingleton() { }
		//static volatile T* Instance() { return (T*)(m_pInstance); }
		public static T Instance() { return m_pInstance; }
		public static bool CreateInstance()
		{
			if (m_pInstance == null)
			{
				m_pInstance = new T();
				return m_pInstance != null;
			}
			return false;
		}

		public static bool DestroyInstance()
		{
			if (m_pInstance != null)
			{
				m_pInstance = default(T);
				return true;
			}
			return false;
		}

		static T m_pInstance;
		protected TSingleton() { }
	}
}
