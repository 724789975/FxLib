#ifndef __GMSession_H__
#define __GMSession_H__

#include <map>
#include <string>
#include "lock.h"
#include "SocketSession.h"
#include "chatdefine.h"
#include "../json/json.h"

class GMSession;
typedef bool (*Operate)(GMSession* pSession, Json::Value& refjReq, Json::Value& refjAck);
class GMSession : public FxSession
{
public:
	GMSession();
	virtual ~GMSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(UINT32 dwErrorNo);
	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual UINT32		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oTextDataHeader; }

	void				GetInfo(Json::Value& refjReq, Json::Value& refjAck);
	void				Broadcast(Json::Value& refjReq, Json::Value& refjAck);
	bool				LoginTest(Json::Value& refjReq, Json::Value& refjAck);
	void				OnLoginSign(std::string szChatIp, unsigned int dwChatPort, unsigned int dwWebSocketChatPort, std::string szPlayerId, std::string szSign);

private:
	TextDataHeader m_oTextDataHeader;
	char m_dataRecvBuf[1024 * 1024];

	std::map<std::string, Operate> m_mapOperate;
};

class GMSessionManager : public IFxSessionFactory
{
public:
	GMSessionManager() {}
	virtual ~GMSessionManager() {}

	virtual FxSession*	CreateSession();

	bool Init() { return true; };

	virtual void Release(FxSession* pSession);

	GMSession& GetGMSession() { return m_oGMSession; }

private:
	GMSession m_oGMSession;

	FxCriticalLock m_oLock;
};

#endif	//!__GMSession_H__
