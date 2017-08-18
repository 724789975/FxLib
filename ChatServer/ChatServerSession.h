#ifndef __CHatSession_H__
#define __CHatSession_H__

#include "SocketSession.h"

class ChatServerSession : public CSocketSession
{
public:
	ChatServerSession();
	virtual ~ChatServerSession();

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);

	virtual void OnRecv(const char* pBuf, UINT32 dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
private:
	char m_szId[32];
};

class ChatServerFactory : public TSingleton<ChatServerFactory>, public IFxSessionFactory
{
public:
	ChatServerFactory();
	virtual ~ChatServerFactory() {}

	virtual FxSession*	CreateSession();

	void Init() {}
	virtual void Release(FxSession* pSession);

	std::set<FxSession*> m_setSessions;

private:
	//	TDynamicPoolEx<CSocketSession> m_poolSessions;

	std::deque<FxSession* > m_listSession;

	IFxLock*			m_pLock;
};


#endif // !__CHatSession_H__
