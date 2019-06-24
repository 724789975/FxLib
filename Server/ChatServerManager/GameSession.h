#ifndef __GameSession_H__
#define __GameSession_H__

#include "lock.h"
#include "SocketSession.h"
#include <map>
#include <string>

class GameSession : public FxSession
{
public:
	GameSession();
	virtual ~GameSession();

	virtual void		OnConnect(void);
	virtual void		OnClose(void);
	virtual void		OnError(unsigned int dwErrorNo);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void);
	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }
	virtual unsigned int		GetRecvSize() { return 64 * 1024; };
	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }

	std::string			GetServerId() { return m_szServerId; }
	void				OnLoginSign(std::string szChatIp, unsigned int dwChatPort, unsigned int dwWebSocketChatPort, std::string szPlayerId, std::string szSign);

private:
	void				OnLogin(const char* pBuf, unsigned int dwLen);
	void				OnPlayerLogin(const char* pBuf, unsigned int dwLen);
private:
	BinaryDataHeader m_oBinaryDataHeader;
	char m_dataRecvBuf[64 * 1024];

	std::string m_szServerId;
};

class GameSessionManager : public IFxSessionFactory
{
public:
	GameSessionManager() {}
	virtual ~GameSessionManager() {}

	virtual FxSession*	CreateSession();

	bool Init();
	void CloseSessions();

	void SetGameSession(std::string szServerId, GameSession* pSession);
	GameSession* GetGameSession(std::string szServerId);

	virtual void Release(FxSession* pSession);
	virtual void Release(std::string szServerId);

private:
	TDynamicPoolEx<GameSession> m_poolSessions;
	std::map<std::string, GameSession*> m_mapGameSessions;
};



#endif // !__CHatSession_H__
