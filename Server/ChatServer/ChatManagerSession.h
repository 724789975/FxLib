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
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

private:
	void OnNotifyChatInfo(const char* pBuf, unsigned int dwLen);
	void OnBroadcastMsg(const char* pBuf, unsigned int dwLen);
	void OnLoginSign(const char* pBuf, unsigned int dwLen);
	void OnLoginSignByGM(const char* pBuf, unsigned int dwLen);
};

#endif // !__ChatManagerSession_H__
