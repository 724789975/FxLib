#ifndef __CHatSession_H__
#define __CHatSession_H__

#include "SocketSession.h"

class ChatSession : public CSocketSession
{
public:
	ChatSession();
	virtual ~ChatSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);

	virtual void OnRecv(const char* pBuf, UINT32 dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
};



#endif // !__CHatSession_H__
