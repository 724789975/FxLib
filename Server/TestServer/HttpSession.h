#ifndef __HttpSession_H__
#define __HttpSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <map>
#include <string>

class CHttpSession : public FxSession
{
public:
	typedef void(*HttpCallBack)(HttpRequestInfo& oHttpRequestInfo, CHttpSession& refHttpSession);

	CHttpSession();
	virtual ~CHttpSession();

	virtual void		OnConnect(void);

	virtual void		OnClose(void);

	virtual void		OnError(unsigned int dwErrorNo);

	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);

	virtual void		Release(void);

	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }

	virtual unsigned int		GetRecvSize() { return 64 * 1024; };

	virtual IFxDataHeader* GetDataHeader() { Assert(0); return NULL; }

	static void			RegistCallBack(std::string szApi, HttpCallBack pfCallBack);

private:
	char m_dataRecvBuf[1024 * 1024];

	static std::map<std::string, HttpCallBack> m_mapCallBacks;
};


class CHttpSessionFactory : public TSingleton<CHttpSessionFactory>, public IFxSessionFactory
{
public:
	CHttpSessionFactory();
	virtual ~CHttpSessionFactory() {}

	virtual FxSession*	CreateSession();

	virtual void Release(FxSession* pSession) { Assert(0); }
	virtual void Release(CHttpSession* pSession);

private:
	TDynamicPoolEx<CHttpSession> m_poolSessions;
};


void HttpCallBackTest(HttpRequestInfo& oHttpRequestInfo, CHttpSession& refHttpSession);
void HttpCallBackIndex(HttpRequestInfo& oHttpRequestInfo, CHttpSession& refHttpSession);

#endif // !__HttpSession_H__
