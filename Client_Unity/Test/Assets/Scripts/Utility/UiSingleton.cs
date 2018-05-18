using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class UiSingleton<T> : MonoBehaviour where T : MonoBehaviour
{
	private static T m_pInstance;

	public static T Instance()
	{
		return m_pInstance;
	}

	public static bool IsValid()
	{
		return m_pInstance != null;
	}

	public static T CreateInstance(UnityEngine.Object go, Transform pTransform)
	{
		if (m_pInstance != null)
		{
			m_pInstance.transform.parent = pTransform;
			return m_pInstance;
		}

		GameObject g1 = Instantiate(go, pTransform) as GameObject;
		m_pInstance = g1.GetComponent<T>();
		if (m_pInstance == null)
		{
			Destroy(g1);
		}
		return m_pInstance;
	}

	public static T CreateInstance(T pInstance)
	{
		if (m_pInstance != null)
		{
			if (m_pInstance != pInstance)
			{
				Destroy(pInstance);
			}
			return m_pInstance;
		}

		m_pInstance = pInstance;
		return m_pInstance;
	}
}
