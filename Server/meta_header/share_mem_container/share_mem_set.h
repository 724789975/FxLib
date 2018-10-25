#ifndef __ShareMemSet_H__
#define __ShareMemSet_H__

#include "share_mem_less.h"

namespace ShareMemory
{
	//K只能为基础类型 且 不能为string, 可以为char[] 
	//K要支持=操作
	template<typename K, unsigned int MAXNUM, class KeyLess = Less<K> >
	class Set
	{
	public:
		typedef const K* Iterator;

		Set() {}

		void Clear()
		{
			m_dwSize = 0;
		}

		Iterator Insert(const K& k)
		{
			if (m_dwSize >= MAXNUM)
			{
				return NULL;
			}

			if (m_dwSize == 0)
			{
				m_oKeys[0] = k;
				++m_dwSize;
				return &m_oKeys[0];
			}

			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			if (Search(k, dwLeftIndex, dwRightIndex) != 0XFFFFFFFF)
			{
				return NULL;
			}
			//放到最前面
			if (m_oLess(k, m_oKeys[dwLeftIndex]))
			{
				memcpy(&m_oKeys[dwLeftIndex + 1], &m_oKeys[dwLeftIndex], (m_dwSize - dwLeftIndex) * sizeof(KVPair));
				m_oKeys[dwLeftIndex] = k;
				++m_dwSize;
				return &m_oKeys[dwLeftIndex];
			}
			//放到最后面
			if (m_oLess(m_oKeys[dwRightIndex], k))
			{
				m_oKeys[m_dwSize] = k;
				++m_dwSize;
				return &m_oKeys[m_dwSize - 1];
			}

			memcpy(&m_oKeys[dwRightIndex + 1], &m_oKeys[dwRightIndex], (m_dwSize - dwRightIndex) * sizeof(KVPair));

			m_oKeys[dwRightIndex] = k;
			++m_dwSize;

			return &m_oKeys[dwRightIndex];
		}

		bool Remove(const K& k)
		{
			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			unsigned int dwIndex = Search(k);
			if (dwIndex == 0XFFFFFFFF)
			{
				return false;
			}

			if (dwIndex == m_dwSize - 1)
			{
				--m_dwSize;
				memset(&m_oKeys[dwIndex], 0, sizeof(K));
				return true;
			}
			memcpy(&m_oKeys[dwIndex], &m_oKeys[dwIndex + 1], (m_dwSize - 1 - dwIndex) * sizeof(K));
			memset(&m_oKeys[m_dwSize - 1], 0, sizeof(K));
			--m_dwSize;
			return true;
		}

		Iterator Find(K k)
		{
			unsigned int dw_index = Search(k);
			if (dw_index == 0XFFFFFFFF)
			{
				return NULL;
			}

			return &m_oKeys[dw_index];
		}

		Iterator Begin()
		{
			return m_oKeys;
		}

		Iterator End()
		{
			return m_oKeys + m_dwSize;
		}

		unsigned int Size() const { return m_dwSize; }
	protected:
	private:

		unsigned int Search(K k)
		{
			if (m_dwSize == 0)
			{
				return 0XFFFFFFFF;
			}

			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			return Search(k, dwLeftIndex, dwRightIndex);
		}

		unsigned int Search(const K& k, unsigned int& dwLeftIndex, unsigned int& dwRightIndex)
		{
			if (m_dwSize == 0)
			{
				return 0XFFFFFFFF;
			}
			//二分法查找
			dwLeftIndex = 0;
			dwRightIndex = m_dwSize - 1;

			unsigned int dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;

			while (dwLeftIndex < dwRightIndex && !m_oEqual(m_oKeys[dwMidIndex], k))
			{
				if (m_oLess(m_oKeys[dwMidIndex], k))
				{
					dwLeftIndex = dwMidIndex + 1;
				}
				else if (m_oLess(k, m_oKeys[dwMidIndex]))
				{
					dwRightIndex = dwMidIndex;
				}
				dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;
			}
			if (m_oEqual(m_oKeys[dwMidIndex], k))
			{
				return dwMidIndex;
			}
			return 0XFFFFFFFF;
		}

	private:
		unsigned int m_dwSize;
		K m_oKeys[MAXNUM];

		KeyLess m_oLess;
		Equal<K, KeyLess> m_oEqual;
	};
};
#endif // !__ShareMemMap_H__
