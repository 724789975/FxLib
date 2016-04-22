#ifndef __DYNAMICPOOLEX_H_2009_1203__
#define __DYNAMICPOOLEX_H_2009_1203__

#include "fxmeta.h"
#include <list>
#include <deque>

class FxFakeLock : public IFxLock
{
public:
    FxFakeLock(){}
    ~FxFakeLock(){}
    
    virtual void Lock(){}
    virtual void UnLock(){}
    virtual void Release(){ delete this; }
};

// nijie modify for placement new 20090428

template <class T>
class TDynamicPoolEx
{
public:
    explicit TDynamicPoolEx(void)
	{
	   	m_dwInitCount = 0;
	   	m_dwGrowCount =0;
        m_dwMaxCount = 0;
        m_dwNowAllCount = 0;
        m_pAllocLock = NULL;
		m_pFreeLock = NULL;
    }

    virtual ~TDynamicPoolEx(void)
    {
//        for(typename std::list<FxShellT*>::iterator itr = m_ListFree.begin(); itr != m_ListFree.end(); itr++)
        for(typename std::deque<FxShellT*>::iterator itr = m_ListFree.begin(); itr != m_ListFree.end(); itr++)
            *itr = NULL;

		m_ListFree.clear();
        
//		for(typename std::list<FxShellT*>::iterator it = m_ListAll.begin(); it != m_ListAll.end(); it++)
		for(typename std::deque<FxShellT*>::iterator it = m_ListAll.begin(); it != m_ListAll.end(); it++)
        {
            // ????malloc / free??????????? nijie 20090428
            // (*itr)->~FxShellT();
            // free(*itr);
            delete [] *it;
            *it = NULL;
        }
		m_ListAll.clear();
        
		if (m_pFreeLock)
		{
			m_pFreeLock->Release();
			m_pFreeLock = NULL;
		}
		
		if (m_pAllocLock)
		{
			m_pAllocLock->Release();
			m_pAllocLock = NULL;
		}
    }

    // Max count???Pool?????????????0?????????????
    bool Init(UINT32 dwInitCount, UINT32 dwGrowCount, bool bSingleThread = false, UINT32 dwMaxCount = 0)
    {
        if(dwMaxCount != 0)
        {
            if(dwMaxCount < dwInitCount)
            {
                return false;
            }
        }
		m_dwMaxCount = dwMaxCount;

        m_dwInitCount = dwInitCount;
        m_dwGrowCount = dwGrowCount;
        
        if (!bSingleThread)
        {
            m_pFreeLock   = FxCreateThreadLock();
            m_pAllocLock  = FxCreateThreadLock();
        }
		else
		{
			m_pAllocLock = new FxFakeLock;
			m_pFreeLock  = new FxFakeLock;
		}

		if (!m_pFreeLock || !m_pAllocLock)
			return false;

        return _AllocT(m_dwInitCount);
    }

    T* FetchObj(void)
    {
        FxShellT* poShellT = NULL;
        m_pFreeLock->Lock();
        if(m_ListFree.empty())
        {
            m_pFreeLock->UnLock();
            if(!_AllocT(m_dwGrowCount))
            {
                return NULL;
            }
            m_pFreeLock->Lock();
        }

        poShellT = m_ListFree.front();
        if (NULL == poShellT)
        {
            m_pFreeLock->UnLock();
            return NULL;
        }

        // ????????trace???????????????????????
        // ????????????????0?????????????????????????????????????????BUG??
        if (0 != poShellT->m_byRef)
        {
            //char szMsg[512];
            //sprintf(szMsg, "poShellT remalloc Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return NULL;
        }
        // ???????????????????
        poShellT->m_byRef++;

        // placement new
        // new(poShellT) FxShellT;
        // ????????????

        m_ListFree.pop_front();
        m_pFreeLock->UnLock();

        return poShellT;
    }

    void ReleaseObj(T* pObj)
    {
        if (NULL == pObj)
        {
            return;
        }

        m_pFreeLock->Lock();
        
        // ????????????????????????????????????????????????????????????
        // ????????????????????????
        FxShellT* poShellT = static_cast<FxShellT*>(pObj);     
        if (NULL == poShellT)
        {
            m_pFreeLock->UnLock();
            return;
        }

        // ?????????????1???????????????
        // ???????????????????????????????????
        if (0 != poShellT->m_byRef - 1)
        {
//             char szMsg[512];
//             sprintf(szMsg, "poShellT Release Error %u\n", poShellT->m_byRef);
            m_pFreeLock->UnLock();
            return;
        }

        // ????????????????1
        poShellT->m_byRef--;

        // ?????????????????
        // poShellT->~FxShellT();
        m_ListFree.push_back(poShellT);
        m_pFreeLock->UnLock();
    }

    size_t GetFreeCount(void)
    {
        size_t n = 0;
        m_pFreeLock->Lock();
        n = m_ListFree.size();
        m_pFreeLock->UnLock();
        return n;
    }

protected:
    bool _AllocT(UINT32 dwCount)
    {
        if (0 == dwCount)
        {
            return false;
        }

        // ??????????????????????
        if(m_dwMaxCount != 0 && m_dwNowAllCount >= m_dwMaxCount)
        {
            return false;
        }

        m_pFreeLock->Lock();
        if(!m_ListFree.empty())
        {
            m_pFreeLock->UnLock();
            return true;
        }

        // ????malloc / free??????????? nijie 20090428
        // FxShellT* poShellT = (FxShellT*) malloc (sizeof(FxShellT) * dwCount);
        UINT32 dwNewCount = dwCount;
        if(m_dwMaxCount != 0)
        {
            if(dwNewCount + m_dwNowAllCount >= m_dwMaxCount)
            {
                dwNewCount = m_dwMaxCount - m_dwNowAllCount;
            }
        }
        FxShellT* poShellT = new FxShellT[dwNewCount];

        if(poShellT == NULL)
        {
            m_pFreeLock->UnLock();
            return false;
        }

        m_dwNowAllCount += dwNewCount;

        // ??????????????????????????????????????
        //if (0 == m_nOffset)
        //{
        //    m_nOffset = int((char*)&(poShellT->m_oT) - (char*)poShellT);
        //}

        for(UINT32 i = 0; i < dwNewCount; ++i)
        {
            // ??????????0
            poShellT[i].m_byRef = 0;
            m_ListFree.push_back(&poShellT[i]);
        }
        m_pFreeLock->UnLock();

        m_pAllocLock->Lock();
        m_ListAll.push_back(poShellT);
        m_pAllocLock->UnLock();

#ifdef _DEBUG
// 		char szMsg[512];
//		sprintf(szMsg, "Free: %u, All: %u\n", m_ListFree.size(), m_ListAll.size());
#endif
        return true;
    }

private:
    // ???????????????
    class FxShellT : public T
    {
    public:
        FxShellT(){}
        ~FxShellT(){}
        UINT8   m_byRef;
        //T       m_oT;
    };

//	std::list<FxShellT*> m_ListFree;
	std::deque<FxShellT*> m_ListFree;
    IFxLock* m_pFreeLock;

//	std::list<FxShellT*> m_ListAll;
	std::deque<FxShellT*> m_ListAll;
    IFxLock* m_pAllocLock;

    UINT32  m_dwInitCount;
    UINT32  m_dwGrowCount;
    UINT32  m_dwMaxCount;

    UINT32  m_dwNowAllCount;
    // ???????????????????linux??g++????????????(?????template?)??vc?????
    // ?????????????????????????????????????????????
    //int     m_nOffset;
};

#endif  // __DYNAMICPOOLEX_H_2009_1203__
