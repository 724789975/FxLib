#ifndef __ShareMemMap_H__
#define __ShareMemMap_H__

#include "share_mem_less.h"

namespace ShareMemory
{
	//K只能为基础类型 且 不能为string, 可以为char[] V必须指定默认构造函数
	//K要支持=操作
	template<typename K, typename V, unsigned int MAXNUM, class KeyLess = Less<K> >
	class Map
	{
		struct KVPair
		{
			K k;
			V v;
		};
	public:
		typedef struct
		{
			const K k;
			V second;
		}*Iterator;

		//typedef KeyStore* Iterator;

		Map() {}

		void Clear()
		{
			m_dwSize = 0;
		}

		V* Alloc(const K& k)
		{
			KVPair* pPair = _Alloc(k);
			if (!pPair)
			{
				return NULL;
			}
			return &(pPair->v);
		}

		Iterator Insert(K k, V v)
		{
			KVPair* pPair = _Alloc(k);
			if (!pPair)
			{
				return NULL;
			}
			memcpy(&pPair->v, &v, sizeof(V));
			return (Iterator)pPair;
		}

		bool Remove(K k)
		{
			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			unsigned int dwIndex = Search(k);
			if (dwIndex == 0XFFFFFFFF)
			{
				return false;
			}

			//m_oPool.ReleaseObj(m_oKeyStores[dwIndex]);
			if (dwIndex == m_dwSize - 1)
			{
				--m_dwSize;
				memset(&m_oKeystores[dwIndex], 0, sizeof(KVPair));
				return true;
			}
			memcpy(&m_oKeystores[dwIndex], &m_oKeystores[dwIndex + 1], (m_dwSize - 1 - dwIndex) * sizeof(KVPair));
			memset(&m_oKeystores[m_dwSize - 1], 0, sizeof(KVPair));
			--m_dwSize;
			return true;
		}

		Iterator Find(K k)
		{
			unsigned int dwIndex = Search(k);
			if (dwIndex == 0XFFFFFFFF)
			{
				return NULL;
			}

			return (Iterator)(&m_oKeystores[dwIndex]);
		}

		Iterator Begin()
		{
			return (Iterator)m_oKeystores;
		}

		Iterator End()
		{
			return (Iterator)m_oKeystores + m_dwSize;
		}

		unsigned int Size() const { return m_dwSize; }

		V& operator [](const K& refK)
		{
			unsigned int dwIndex = Search(refK);
			if (dwIndex == 0XFFFFFFFF)
			{
				KVPair* pPair = _Alloc(refK);
				new(&(pPair->v)) V();
				return pPair->v;
			}

			return m_oKeystores[dwIndex].v;
		}
	protected:
	private:
		KVPair* _Alloc(const K& k)
		{
			if (m_dwSize >= MAXNUM)
			{
				return nullptr;
			}

			if (m_dwSize == 0)
			{
				//memcpy(&m_o_keystores[0].v, &v, sizeof(v));
				m_oKeystores[0].k = k;
				++m_dwSize;
				return &m_oKeystores[0];
			}

			unsigned int dwLeftIndex = 0;
			unsigned int dwRightIndex = 0;

			if (Search(k, dwLeftIndex, dwRightIndex) != 0XFFFFFFFF)
			{
				return nullptr;
			}
			//放到最前面
			if (m_oLess(k, m_oKeystores[dwLeftIndex].k))
			{
				memcpy(&m_oKeystores[dwLeftIndex + 1], &m_oKeystores[dwLeftIndex], (m_dwSize - dwLeftIndex) * sizeof(KVPair));
				//memcpy(&m_o_keystores[dw_left_index].v, &v, sizeof(v));
				m_oKeystores[dwLeftIndex].k = k;
				++m_dwSize;
				return &m_oKeystores[dwLeftIndex];
			}
			//放到最后面
			if (m_oLess(m_oKeystores[dwRightIndex].k, k))
			{
				//memcpy(&m_o_keystores[m_dw_size].v, &v, sizeof(v));
				m_oKeystores[m_dwSize].k = k;
				++m_dwSize;
				return &m_oKeystores[m_dwSize - 1];
			}

			memcpy(&m_oKeystores[dwRightIndex + 1], &m_oKeystores[dwRightIndex], (m_dwSize - dwRightIndex) * sizeof(KVPair));

			//m_o_keystores[dw_right_index].v = v;
			m_oKeystores[dwRightIndex].k = k;
			++m_dwSize;

			return &m_oKeystores[dwRightIndex];
		}

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

		unsigned int Search(K k, unsigned int& dwLeftIndex, unsigned int& dwRightIndex)
		{
			if (m_dwSize == 0)
			{
				return 0XFFFFFFFF;
			}
			//二分法查找
			dwLeftIndex = 0;
			dwRightIndex = m_dwSize - 1;

			unsigned int dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;

			while (dwLeftIndex < dwRightIndex && !m_oEqual(m_oKeystores[dwMidIndex].k, k))
			{
				if (m_oLess(m_oKeystores[dwMidIndex].k, k))
				{
					dwLeftIndex = dwMidIndex + 1;
				}
				else if (m_oLess(k, m_oKeystores[dwMidIndex].k))
				{
					dwRightIndex = dwMidIndex;
				}
				dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;
			}
			if (m_oEqual(m_oKeystores[dwMidIndex].k, k))
			{
				return dwMidIndex;
			}
			return 0XFFFFFFFF;
		}

	private:
		unsigned int m_dwSize;
		KVPair m_oKeystores[MAXNUM];

		KeyLess m_oLess;
		Equal<K, KeyLess> m_oEqual;
	};

};

#endif // !__ShareMemMap_H__
