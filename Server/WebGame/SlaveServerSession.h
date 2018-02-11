#ifndef __CSlaveServerSession_H__
#define __CSlaveServerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>

class CSlaveServerSession : public FxSession
{
public:
	CSlaveServerSession();
	~CSlaveServerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	//void				OnRequestGameManagerInfo(const char* pBuf, UINT32 dwLen);
private:
	char m_dataRecvBuf[1024 * 1024];
};

#endif // !__CSlaveServerSession_H__
