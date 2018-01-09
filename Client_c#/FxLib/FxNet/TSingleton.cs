using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FxNet
{
	public abstract class TSingleton<T> where T:new()
	{
		~TSingleton() { }
		//static volatile T* Instance() { return (T*)(m_pInstance); }
		public static T Instance() { return m_pInstance; }
		static bool CreateInstance()
		{
			if (m_pInstance == null)
			{
				m_pInstance = new T();
				return m_pInstance != null;
			}
			return false;
		}

		static bool DestroyInstance()
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
