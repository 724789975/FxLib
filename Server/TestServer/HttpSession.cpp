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
		std::string szContent = "<!DOCTYPE html>"
			"<html>\r\n"
			"<head>\r\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
			"<title>Test</title>\r\n"
			"<body>\r\n"
			"404 NOT FOUND\r\n"
			"</body>\r\n"
			"</html>\r\n";

		char szBuf[1024 * 16] = { 0 };
		sprintf(szBuf, "HTTP/1.1 404 ERROR\r\n"
			"Server: Test\r\n"
			"Content-Type: text/html\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s", szContent.size(), szContent.c_str());
		Send(szBuf, strlen(szBuf));
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
	std::string szBuf = "HTTP/1.1 200 OK\r\n"
		"Server: Test\r\n"
		"Content-Type: text/html\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"\r\n"
		"<!DOCTYPE html>"
		"<html>\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
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

