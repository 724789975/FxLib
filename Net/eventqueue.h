#ifndef __THRDQUEUE_H_NICK_2009_1203__
#define __THRDQUEUE_H_NICK_2009_1203__

#include "fxmeta.h"
#include <stdlib.h>
#include "lock.h"

template <class T>
class TEventQueue
{
public:
    TEventQueue(void)
    {
    	m_nSize = 0;
        m_nOutCursor    = 0;
        m_nInCursor     = 0;
        m_nOutCursor    = 0;
        m_poArrayT      = NULL;
    };

    virtual ~TEventQueue(void)
    {
        if (NULL != m_poArrayT)
        {
            free(m_poArrayT);
            m_poArrayT = NULL;
        }
    };

	inline bool Init(INT32 nMax)
    {
        if (nMax != m_nSize)
        {
            m_poArrayT = (T*)realloc(m_poArrayT, sizeof(T) * nMax);
            if (NULL == m_poArrayT)
            {
                return false;
            }
        }
        m_nOutCursor    = 0;
        m_nInCursor     = 0;
        m_nSize         = nMax;
        return true;
    }

    inline int GetSize()
    {
        return m_nSize;
    }

	inline bool PushBack(T& ot)
    {
        m_poLock.Lock();
        int nSize = m_nInCursor + m_nSize - m_nOutCursor;
        nSize = nSize >= m_nSize ? (nSize - m_nSize) : nSize;
        if (m_nSize - 1 <= nSize)
        {
            m_poLock.UnLock();
            return false;
        }
        m_poArrayT[m_nInCursor] = ot;
        if(++m_nInCursor >= m_nSize)
            m_nInCursor = 0;

        m_poLock.UnLock();
        return true;
    }

    inline T* PopFront()
    {
        // 
        // ȡ�¼������ڵ��߳��н��У���ʱ�Ժ�ʹ�ö��̴߳����¼���
        // ��ôҲ�ܱ�֤���¼�����ͬʱֻ����һ���߳���ȡ�¼�
        // ��������ֻҪ��֤�����Ƴ�������Ҽ���
        // ������ѭ�����еķ�ʽ���߼��������������������Ƴ���������
        // ���Բ��ü�����������Ϊ�кܶ��̻߳����룬Ҳͬʱ������һ���߳����� ȡ�¼�
        // ������������Ӱ������
        // 

        //m_poLock->Lock();
        int nSize = m_nInCursor + m_nSize - m_nOutCursor;
        nSize = nSize >= m_nSize ? (nSize - m_nSize) : nSize;
        if(0 == nSize)
        {
            //m_poLock->UnLock();
            return NULL;
        }

        T* ptr = &m_poArrayT[m_nOutCursor];
        if(++m_nOutCursor >= m_nSize)
            m_nOutCursor = 0;

        //m_poLock->UnLock();

        return ptr;
    }

private:
    T*          m_poArrayT;
    int      	m_nSize;
	int      	m_nInCursor;
	int      	m_nOutCursor;
	FxCriticalLock    m_poLock;
};

#endif  // __THRDQUEUE_H_NICK_2009_1203__

