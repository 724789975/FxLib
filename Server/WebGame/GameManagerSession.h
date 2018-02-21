#ifndef __CGameManagerSession_H__
#define __CGameManagerSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>
#include "SocketSession.h"

class CGameManagerSession : public FxSession
{
public:
	CGameManagerSession();
	virtual ~CGameManagerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	void				OnGameNotifyGameManagerInfo(const char* pBuf, UINT32 dwLen);
private:
	char m_dataRecvBuf[1024 * 1024];
};

class CBinaryGameManagerSession : public CGameManagerSession
{
public:
	CBinaryGameManagerSession();
	~CBinaryGameManagerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;

};

#endif	//!__CGameManagerSession_H__