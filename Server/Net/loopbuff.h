#ifndef __LOOPBUFF_H_NIJIE_2009_0825__
#define __LOOPBUFF_H_NIJIE_2009_0825__

#include "fxmeta.h"
#include "dynamicpoolex.h"

class FxLoopBuff
{
public:
	 FxLoopBuff();
	 virtual ~FxLoopBuff();

	 bool					Init(int nLen);
	 bool					IsEmpty();
	 void					Clear();
	 int					GetTotalLen();
	 int					GetFreeLen();
	 int					GetUseLen();
	 int					GetInCursorPtr(char*& pBuf);
	 int					GetOutCursorPtr(char*& pBuf);
	 int					GetUsedCursorPtr(char*& pBuf);
	 bool					PushBuff(const char* pInBuff, int nLen);
	 bool					PopBuff(char* pOutBuff, int nLen);
	 bool					DiscardBuff(int nLen);
	 bool					CostBuff(int nLen);
	 bool					CostUsedBuff(int nLen);

private:
	 char*					__GetInCursorPtr();
	 char*					__GetOutCursorPtr();

private:
	 int					m_nTotalLen;			// 分配的总长度//
	 int					m_nFreeLen;			// 当前空闲长度//
	 int					m_nInCursor;			// 当前放入的游标//
	 int					m_nOutCursor;		 // 当前取出的游标//
	 int					m_nUseCursor;
	 char*					m_pszBuff;			 // Buff
	 bool					m_bLoop;
	 bool					m_bUseLoop;
	 FxCriticalLock			m_oLock;
};


//////////////////////////////////////////////////////////////////////////

class FxLoopBuffMgr : public TSingleton<FxLoopBuffMgr>
{
public:
	 FxLoopBuffMgr();
	 ~FxLoopBuffMgr();
	 //DECLARE_SINGLETON(FxLoopBuffMgr);

	 bool					 Init(unsigned int dwCount);
	 FxLoopBuff*			 Fetch();
	 bool					 Release(FxLoopBuff* pBuff);

private:
	 TDynamicPoolEx<FxLoopBuff>	m_oBuffPool;
};

#endif	// __LOOPBUFF_H_NIJIE_2009_0825__

