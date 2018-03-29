#include "HttpSession.h"

CHttpSession::CHttpSession()
{
}


CHttpSession::~CHttpSession()
{
}

void CHttpSession::OnConnect(void)
{
}

void CHttpSession::OnClose(void)
{
}

void CHttpSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

void CHttpSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), pBuf);

	HttpRequestInfo oInfo = { 0 };
	HttpHelp::parse_http_request(m_dataRecvBuf, dwLen, &oInfo);

	std::map<std::string, HttpCallBack>::iterator it = m_mapCallBacks.find(oInfo.request_uri);
	if (it == m_mapCallBacks.end())
	{
		LogExe(LogLv_Error, "can't find api : %s", oInfo.request_uri);
		std::string szBuf = "{\"error\":\"没找到对应api\"}";
		Send(szBuf.c_str(), szBuf.size());
		return;
	}
	it->second(oInfo, *this);
	Close();
}

void CHttpSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}


void CHttpSession::RegistCallBack(std::string szApi, HttpCallBack pfCallBack)
{
	m_mapCallBacks[szApi] = pfCallBack;
}

std::map<std::string, CHttpSession::HttpCallBack> CHttpSession::m_mapCallBacks;

CHttpSessionFactory::CHttpSessionFactory()
{
	m_poolSessions.Init(64, 64);
}

FxSession*	CHttpSessionFactory::CreateSession()
{
	return m_poolSessions.FetchObj();
}

void CHttpSessionFactory::Release(CHttpSession* pSession)
{
	m_poolSessions.ReleaseObj(pSession);
}

void HttpCallBackTest(HttpRequestInfo& oHttpRequestInfo, CHttpSession& refHttpSession)
{
	std::string szBuf = "<!DOCTYPE html PUBLIC \" -//W3C//DTD XHTML 1.0 Stict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n"
		"<html xmlns = \"http://www.w3.org/1999/xhtml\" lang = \"zh-CN\">\r\n"
		"<html>\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content - Type\" content=\"text / html; charset = \"UTF-8\">\r\n"
		"<title>Test</title>\r\n"
		"<body>\r\n"
		"</body>\r\n"
		"</html>\r\n";

	if (!refHttpSession.Send(szBuf.c_str(), szBuf.size()))
	{
		refHttpSession.Close();
	}
}

class Init
{
public:
	Init()
	{
		CHttpSession::RegistCallBack("/test", HttpCallBackTest);
	}
	~Init(){}

private:

};

Init oInit;

