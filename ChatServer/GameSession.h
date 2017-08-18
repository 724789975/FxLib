#ifndef __CHatSession_H__
#define __CHatSession_H__

#include "SocketSession.h"

class GameSession : public CSocketSession
{
public:
	GameSession();
	virtual ~GameSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);

	virtual void OnRecv(const char* pBuf, UINT32 dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
};



#endif // !__CHatSession_H__
