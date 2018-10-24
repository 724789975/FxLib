#ifndef __ShareMemMap_H__
#define __ShareMemMap_H__

//K只能为基础类型 且 不能为string, 可以为char[] V必须指定默认构造函数
template<typename K, typename V, unsigned int MAXNUM>
class MapPool
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
	}* Iterator;

	//typedef KeyStore* Iterator;

	MapPool() {}

	bool create()
	{
		//memset(this, 0, sizeof(MapPool));
		//return true;
	}

	V* Alloc(const K& k)
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
		if (m_oKeystores[dwLeftIndex].k > k)
		{
			memcpy(&m_oKeystores[dwLeftIndex + 1], &m_oKeystores[dwLeftIndex], (m_dwSize - dwLeftIndex) * sizeof(KVPair));
			//memcpy(&m_o_keystores[dw_left_index].v, &v, sizeof(v));
			m_oKeystores[dwLeftIndex].k = k;
			++m_dwSize;
			return &m_oKeystores[dwLeftIndex];
		}
		//放到最后面
		if (m_oKeystores[dwRightIndex].k < k)
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

	Iterator Insert(K k, V v)
	{
		return NULL;
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
		unsigned int dw_index = Search(k);
		if (dw_index == 0XFFFFFFFF)
		{
			return nullptr;
		}

		//new(&m_oKeystores[dw_index].v) V();
		return (Iterator)(&m_oKeystores[dw_index]);
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

	V& operator [](K k)
	{
		unsigned int dw_index = Search(k);
		if (dw_index == 0XFFFFFFFF)
		{
			return Insert(k, V())->v;
		}

		return m_oKeystores[dw_index].v;
	}

	const V& operator [](K k) const
	{
		unsigned int dw_index = Search(k);
		if (dw_index == 0XFFFFFFFF)
		{
			return Insert(k, V())->v;
		}

		new(&m_oKeystores[dw_index].v) V();
		return m_oKeystores[dw_index].v;
	}
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

		while (dwLeftIndex < dwRightIndex && m_oKeystores[dwMidIndex].k != k)
		{
			if (m_oKeystores[dwMidIndex].k < k)
			{
				dwLeftIndex = dwMidIndex + 1;
			}
			else if (m_oKeystores[dwMidIndex].k > k)
			{
				dwRightIndex = dwMidIndex;
			}
			dwMidIndex = (dwLeftIndex + dwRightIndex) / 2;
		}
		if (m_oKeystores[dwMidIndex].k == k)
		{
			return dwMidIndex;
		}
		return 0XFFFFFFFF;
	}

private:
	unsigned int m_dwSize;
	KVPair m_oKeystores[MAXNUM];
};

#endif // !__ShareMemMap_H__
