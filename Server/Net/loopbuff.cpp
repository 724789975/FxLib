#include "loopbuff.h"
#include <malloc.h>
#include <memory.h>
#include "lock.h"

FxLoopBuff::FxLoopBuff()
{
    m_nTotalLen     = 0;
    m_nFreeLen      = 0;
    m_nInCursor     = 0;
    m_nOutCursor    = 0;
    m_nUseCursor    = 0;
    m_pszBuff       = NULL;
    m_bLoop         = false;
    m_bUseLoop      = false;
}

FxLoopBuff::~FxLoopBuff()
{
    if (NULL != m_pszBuff)
    {
        free(m_pszBuff);
        m_pszBuff = NULL;
    }

    m_nTotalLen     = 0;
    m_nFreeLen      = 0;
    m_nInCursor     = 0;
    m_nOutCursor    = 0;
    m_bLoop         = false;
    m_nUseCursor    = 0;
    m_bUseLoop      = false;
}

bool FxLoopBuff::Init(int nLen)
{
	char* pTem = m_pszBuff;
    if (nLen != m_nTotalLen)
    {
        m_pszBuff = (char*)realloc(m_pszBuff, nLen);
    }

    if (NULL == m_pszBuff)
    {
		// realloc 失败 以前开辟过空间的话 会内存泄漏
		m_pszBuff = pTem;
        return false;
    }
    
    m_nTotalLen    = nLen;
    m_nFreeLen     = m_nTotalLen;
    m_nUseCursor   = 0;
    m_nInCursor    = 0;
    m_nOutCursor   = 0;
    m_bLoop        = false;
    m_bUseLoop     = false;
    return true;
}

int FxLoopBuff::GetTotalLen()
{
    return m_nTotalLen;
}

int FxLoopBuff::GetFreeLen()
{
    return m_nFreeLen;
}

int FxLoopBuff::GetUseLen()
{
	FxLockImp oLock(&m_oLock);
    if (m_nFreeLen == m_nTotalLen)
    {
        return 0;
    }

    int nRet = 0;

    if (m_bUseLoop)
    {
        nRet = m_nTotalLen + m_nInCursor - m_nUseCursor;
    }
    else
    {
        nRet = m_nInCursor - m_nUseCursor;
    }

    return nRet;
}

int FxLoopBuff::GetInCursorPtr(char*& pBuf)
{
	FxLockImp oLock(&m_oLock);

    if (0 == m_nFreeLen)
    {
        pBuf = NULL;
        return 0;
    }

    pBuf = m_pszBuff + m_nInCursor;
    int nRet = 0;

    if (m_bLoop)
    {
        nRet = m_nOutCursor - m_nInCursor;
    }
    else
    {
        nRet = m_nTotalLen - m_nInCursor;
    }

    return nRet;
}

int FxLoopBuff::GetOutCursorPtr(char*& pBuf)
{
	FxLockImp oLock(&m_oLock);
    if (m_nTotalLen == m_nFreeLen)
    {
        pBuf = NULL;
        return 0;
    }

    pBuf = m_pszBuff + m_nOutCursor;

    int nRet = 0;
    if (m_bLoop)
    {
        nRet = m_nTotalLen - m_nOutCursor;
    }
    else
    {
        nRet = m_nInCursor - m_nOutCursor;
    }

    return nRet;
}

int FxLoopBuff::GetUsedCursorPtr(char*& pBuf)
{
	FxLockImp oLock(&m_oLock);
    if (m_nTotalLen == m_nFreeLen)
    {
        pBuf = NULL;
        return 0;
    }

    pBuf = m_pszBuff + m_nUseCursor;
    int nRet = 0;

    if (m_bUseLoop)
    {
        nRet = m_nTotalLen - m_nUseCursor;
    }
    else
    {
        nRet = m_nInCursor - m_nUseCursor;
    }

    return nRet;
}

bool FxLoopBuff::PushBuff(const char* pInBuff, int nLen)
{
	FxLockImp oLock(&m_oLock);
    if (0 >= nLen)
    {
        return false;
    }

    if (nLen > m_nFreeLen)
    {
        // 超出空闲字节长度
        return false;
    }

    if (nLen + m_nInCursor == m_nTotalLen)
    {
        memcpy(__GetInCursorPtr(), pInBuff, nLen);
        m_nInCursor = 0;
        m_bLoop = true;
        m_bUseLoop = true;
    }
    else if (nLen + m_nInCursor < m_nTotalLen)
    {
        memcpy(__GetInCursorPtr(), pInBuff, nLen);
        m_nInCursor += nLen;
    }
    else
    {
        int nBuff0 = m_nTotalLen - m_nInCursor;
        memcpy(__GetInCursorPtr(), pInBuff, nBuff0);

        int nBuff1 = nLen - nBuff0;
        memcpy(m_pszBuff, (pInBuff + nBuff0), nBuff1);
        m_nInCursor = nBuff1;
        m_bLoop = true;
        m_bUseLoop = true;
    }

    m_nFreeLen -= nLen;
    return true;
}

bool FxLoopBuff::PopBuff(char* pOutBuff, int nLen)
{
	FxLockImp oLock(&m_oLock);
    if (0 >= nLen)
    {
        return false;
    }

    if (nLen + m_nFreeLen > m_nTotalLen)
    {
        // 超出buff中已用长度
        return false;
    }

    if (nLen + m_nOutCursor == m_nTotalLen)
    {
        memcpy(pOutBuff, __GetOutCursorPtr(), nLen);
        m_nOutCursor = 0;
        m_bLoop = false;
    }
    else if (nLen + m_nOutCursor < m_nTotalLen)
    {
        memcpy(pOutBuff, __GetOutCursorPtr(), nLen);
        m_nOutCursor += nLen;
    }
    else
    {
        int nBuff0 = m_nTotalLen - m_nOutCursor;
        memcpy(pOutBuff, __GetOutCursorPtr(), nBuff0);

        int nBuff1 = nLen - nBuff0;
        memcpy((pOutBuff + nBuff0), m_pszBuff, nBuff1);
        m_nOutCursor = nBuff1;
        m_bLoop = false;
    }

    m_nFreeLen += nLen;
    return true;
}

bool FxLoopBuff::DiscardBuff(int nLen)
{
	FxLockImp oLock(&m_oLock);
    if (0 >= nLen)
    {
        return false;
    }

    if (nLen + m_nFreeLen > m_nTotalLen)
    {
        // 超出buff中已用长度
        return false;
    }

    if (nLen + m_nOutCursor < m_nTotalLen)
    {
        m_nOutCursor += nLen;
    }
    else
    {
        m_nOutCursor = nLen + m_nOutCursor - m_nTotalLen;
        m_bLoop = false;
    }
    
    m_nFreeLen += nLen;
    return true;
}

bool FxLoopBuff::CostBuff(int nLen)
{
	FxLockImp oLock(&m_oLock);
    if (0 >= nLen)
    {
        return false;
    }

    if (nLen > m_nFreeLen)
    {
        // 超出空闲字节长度
        return false;
    }

    if (nLen + m_nInCursor < m_nTotalLen)
    {
        m_nInCursor += nLen;
    }
    else
    {
        m_nInCursor = nLen + m_nInCursor - m_nTotalLen;
        m_bLoop = true;
        m_bUseLoop = true;
    }

    m_nFreeLen -= nLen;
    return true;
}

bool FxLoopBuff::CostUsedBuff(int nLen)
{
	FxLockImp oLock(&m_oLock);
    if (0 >= nLen)
    {
        return false;
    }

    if (m_nFreeLen + nLen > m_nTotalLen)
    {
        // 超出buff中空闲长度
        return false;
    }

    int nUseCursor = 0;

    if (m_bUseLoop)
    {
        if (nLen + m_nUseCursor < m_nTotalLen)
        {
            m_nUseCursor += nLen;
        }
        else
        {
            nUseCursor = m_nUseCursor + nLen - m_nTotalLen;
            if (nUseCursor > m_nInCursor)
            {
                return false;
            }
            m_nUseCursor = nUseCursor;
            m_bUseLoop = false;
        }
    }
    else
    {
        if (nLen + m_nUseCursor > m_nInCursor)
        {
            return false;
        }

        m_nUseCursor += nLen;
    }

    return true;
}

char* FxLoopBuff::__GetInCursorPtr()
{
    if (0 == m_nFreeLen)
    {
        return NULL;
    }

    if (m_nInCursor == m_nTotalLen)
    {
        m_nInCursor = 0;
    }
    return m_pszBuff + m_nInCursor;
}

char* FxLoopBuff::__GetOutCursorPtr()
{
    if (m_nTotalLen == m_nFreeLen)
    {
        return NULL;
    }

    if (m_nOutCursor == m_nTotalLen)
    {
        m_nOutCursor = 0;
    }
    return m_pszBuff + m_nOutCursor;
}

bool FxLoopBuff::IsEmpty()
{
    return m_nTotalLen == m_nFreeLen;
}

void FxLoopBuff::Clear()
{
    m_nUseCursor   = 0;
    m_nInCursor    = 0;
    m_nOutCursor   = 0;
    m_bLoop        = false;
    m_bUseLoop     = false;
}
//////////////////////////////////////////////////////////////////////////

//IMPLEMENT_SINGLETON(FxLoopBuffMgr);

FxLoopBuffMgr::FxLoopBuffMgr()
{

}

FxLoopBuffMgr::~FxLoopBuffMgr()
{

}

bool FxLoopBuffMgr::Init(unsigned int dwCount)
{
    m_oBuffPool.Init(dwCount, dwCount / 2);
    return true;
}

FxLoopBuff* FxLoopBuffMgr::Fetch()
{
    return m_oBuffPool.FetchObj();
}

bool FxLoopBuffMgr::Release(FxLoopBuff* pBuff)
{
    m_oBuffPool.ReleaseObj(pBuff);
    return true;
}

