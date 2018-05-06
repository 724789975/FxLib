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

	public static T CreateInstance(string szPrefabPath, Transform pTransform)
	{
		if (m_pInstance != null)
		{
			m_pInstance.transform.parent = pTransform;
			return m_pInstance;
		}

		m_pInstance = Instantiate(Resources.Load<T>(szPrefabPath), pTransform);
		return m_pInstance;
	}
}
