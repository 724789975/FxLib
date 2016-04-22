#ifndef __LOOPBUFF_H_NIJIE_2009_0825__
#define __LOOPBUFF_H_NIJIE_2009_0825__

#include "fxmeta.h"
#include "dynamicpoolex.h"

class FxLoopBuff
{
public:
	 FxLoopBuff();
	 virtual ~FxLoopBuff();

	 bool					 Init(int nLen);
	 bool					 IsEmpty();
	 void					 Clear();
	 int						GetTotalLen();
	 int						GetFreeLen();
	 int						GetUseLen();
	 int						GetInCursorPtr(char*& pBuf);
	 int						GetOutCursorPtr(char*& pBuf);
	 int						GetUsedCursorPtr(char*& pBuf);
	 bool					 PushBuff(const char* pInBuff, int nLen);
	 bool					 PopBuff(char* pOutBuff, int nLen);
	 bool					 DiscardBuff(int nLen);
	 bool					 CostBuff(int nLen);
	 bool					 CostUsedBuff(int nLen);

private:
	 char*					__GetInCursorPtr();
	 char*					__GetOutCursorPtr();

private:
	 int						m_nTotalLen;			// ������ܳ���//
	 int						m_nFreeLen;			// ��ǰ���г���//
	 int						m_nInCursor;			// ��ǰ������α�//
	 int						m_nOutCursor;		 // ��ǰȡ�����α�//
	 int						m_nUseCursor;
	 char*					m_pszBuff;			 // Buff
	 bool					m_bLoop;
	 bool					m_bUseLoop;
	 IFxLock*				m_poLock;
};


//////////////////////////////////////////////////////////////////////////

class FxLoopBuffMgr
{
public:
	 FxLoopBuffMgr();
	 ~FxLoopBuffMgr();
	 DECLARE_SINGLETON(FxLoopBuffMgr);

	 bool					 Init(UINT32 dwCount);
	 FxLoopBuff*			 Fetch();
	 bool					 Release(FxLoopBuff* pBuff);

private:
	 TDynamicPoolEx<FxLoopBuff>	m_oBuffPool;
};

#endif	// __LOOPBUFF_H_NIJIE_2009_0825__

