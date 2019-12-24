#ifndef __ShareMemSet_H__
#define __ShareMemSet_H__

#include "share_mem_less.h"
#include <exception>

namespace ShareMemory
{
	//K只能为基础类型 且 不能为string, 可以为char[] 
	//K要支持=操作
	template<typename K, unsigned int MAXNUM, class KeyLess = Less<K> >
	class Set
	{
	public:
		typedef const K* iterator;

		Set() : m_dwSize(0)
		{
			m_pKeys = new K[MAXNUM];
			if (!m_pKeys)
			{
				throw(std::exception())
			}
		}

		void clear()
		{
			m_dwSize = 0;
		}

		iterator insert(const K& k)
		{
			if (m_dwSize >= MAXNUM)
			{
				return end();
			}

			if (m_dwSize == 0)
			{
				m_pKeys[0] = k;
				++m_dwSize;
				return &m_pKeys[0];
			}

			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			int dwIndexTemp = Search(k, dwLeftIndex, dwRightIndex);
			if (dwIndexTemp != 0XFFFFFFFF)
			{
				return &m_pKeys[dwIndexTemp];
			}
			//放到最前面
			if (m_oLess(k, m_pKeys[dwLeftIndex]))
			{
				memmove(m_pKeys + dwLeftIndex + 1, m_pKeys + dwLeftIndex, (m_dwSize - dwLeftIndex) * sizeof(KVPair));
				m_pKeys[dwLeftIndex] = k;
				++m_dwSize;
				return m_pKeys + dwLeftIndex;
			}
			//放到最后面
			if (m_oLess(m_pKeys[dwRightIndex], k))
			{
				m_pKeys[m_dwSize] = k;
				++m_dwSize;
				return m_pKeys + m_dwSize - 1;
			}

			memmove(m_pKeys + dwRightIndex + 1, m_pKeys + dwRightIndex, (m_dwSize - dwRightIndex) * sizeof(KVPair));

			m_pKeys[dwRightIndex] = k;
			++m_dwSize;

			return m_pKeys + dwRightIndex;
		}

		bool empty()const
		{
			return this->m_dwSize == 0;
		}

		iterator erase(iterator k)
		{
			return erase(*k);
		}

		iterator erase(const K& k)
		{
			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			unsigned int dwIndex = Search(k);
			if (dwIndex == 0XFFFFFFFF)
			{
				return end();
			}

			if (dwIndex == m_dwSize - 1)
			{
				--m_dwSize;
				memset(m_pKeys + dwIndex, 0, sizeof(K));
				return m_pKeys + dwIndex;
			}
			memmove(m_pKeys + dwIndex, m_pKeys + dwIndex + 1, (m_dwSize - 1 - dwIndex) * sizeof(K));
			memset(m_pKeys + m_dwSize - 1, 0, sizeof(K));
			--m_dwSize;
			return m_pKeys + dwIndex;
		}

		iterator find(K k)
		{
			unsigned int dw_index = Search(k);
			if (dw_index == 0XFFFFFFFF)
			{
				return end();
			}

			return m_pKeys + dw_index;
		}

		iterator begin() const
		{
			return m_pKeys;
		}

		iterator end() const
		{
			return m_pKeys + m_dwSize;
		}

		unsigned int size() const { return m_dwSize; }
	protected:
	private:

		unsigned int Search(const K& k) const
		{
			if (m_dwSize == 0)
			{
				return 0XFFFFFFFF;
			}

			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			return Search(k, dwLeftIndex, dwRightIndex);
		}

		unsigned int Search(const K& k, unsigned int& dwLeftIndex, unsigned int& dwRightIndex) const
		{
			if (m_dwSize == 0)
			{
				return 0XFFFFFFFF;
			}
			//二分法查找
			dwLeftIndex = 0;
			dwRightIndex = m_dwSize - 1;

			unsigned int dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;

			while (dwLeftIndex < dwRightIndex && !m_oEqual(m_pKeys[dwMidIndex], k))
			{
				if (m_oLess(m_pKeys[dwMidIndex], k))
				{
					dwLeftIndex = dwMidIndex + 1;
				}
				else if (m_oLess(k, m_pKeys[dwMidIndex]))
				{
					dwRightIndex = dwMidIndex;
				}
				dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;
			}
			if (m_oEqual(m_pKeys[dwMidIndex], k))
			{
				return dwMidIndex;
			}
			return 0XFFFFFFFF;
		}

	private:
		unsigned int m_dwSize;
		K* m_pKeys;

		KeyLess m_oLess;
		Equal<K, KeyLess> m_oEqual;
	};
};
#endif // !__ShareMemMap_H__
