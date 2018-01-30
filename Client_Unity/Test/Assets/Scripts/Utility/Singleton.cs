using System;
using UnityEngine;

public class Singleton<T>
{
    private static T m_pInstance;
    private static readonly object lock_ = new object();

    protected Singleton()
    {
    }

    public static T Instance()
    {
        if (m_pInstance == null)
        {
            lock (lock_)
            {
                if (m_pInstance == null)
                {
                    m_pInstance = (T) Activator.CreateInstance(typeof (T));
                }
            }
        }
        return m_pInstance;
    }
}

public class SingletonObject<T> : MonoBehaviour where T : MonoBehaviour
{
    private static T m_pInstance;

	public T Instance()
	{
		return m_pInstance;
	}

    public static bool IsValid()
    {
        return m_pInstance != null;
    }

	public bool CreateInstance(T pInstance)
	{
		if (m_pInstance != null)
		{
			Destroy(pInstance);
			return false;
		}

		m_pInstance = pInstance;
		return true;
	}
}