#ifndef __ChatManagerSession_H__
#define __ChatManagerSession_H__

#include "SocketSession.h"

class ChatManagerSession : public FxSession
{
public:
	ChatManagerSession();
	~ChatManagerSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

private:
	void OnNotifyChatInfo(const char* pBuf, UINT32 dwLen);
};

#endif // !__ChatManagerSession_H__
