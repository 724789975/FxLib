#include "HttpSession.h"
#include <fstream>
#include <sstream>

const static char* g_szResponse =
	"HTTP/1.1 %d %s\r\n"
	"Server: Test\r\n"
	"Content-Type: text/html\r\n"
	"Access-Control-Allow-Origin: *\r\n"
	"Date: %s\r\n"
	"Content-Length: %d\r\n"
	"\r\n"
	"%s";

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
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, connect id : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), GetConnectId());
}

void CHttpSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, connect id : %d, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), GetConnectId(), dwErrorNo);
}

void CHttpSession::OnRecv(const char* pBuf, unsigned int dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect id : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), GetConnectId(), pBuf);

	HttpRequestInfo oInfo = { 0 };
	if (0 >= HttpHelp::parse_http_request(m_dataRecvBuf, dwLen, &oInfo))
	{
		LogExe(LogLv_Warn, "parse_http_request error!!!");
		return;
	}

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
		sprintf(szBuf, g_szResponse, 404, HttpHelp::mg_get_response_code_text(404), GetTimeHandler()->GetTimeStr(), szContent.size(), szContent.c_str());
		Send(szBuf, strlen(szBuf));
		return;
	}
	it->second(oInfo, *this);
	//Close();
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
	char szBuf[1024 * 16] = { 0 };
	std::string szContent =
		"<!DOCTYPE html>"
		"<html>\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
		"<title>Test</title>\r\n"
		"<body>\r\n"
		"test</br>\r\n"
		"</body>\r\n"
		"</html>\r\n";

	sprintf(szBuf, g_szResponse, 200, HttpHelp::mg_get_response_code_text(200), GetTimeHandler()->GetTimeStr(), szContent.size(), szContent.c_str());

	if (!refHttpSession.Send(szBuf, strlen(szBuf)))
	{
		//refHttpSession.Close();
	}
}

void HttpCallBackIndex(HttpRequestInfo & oHttpRequestInfo, CHttpSession & refHttpSession)
{
	std::ifstream t1("index.html");
	std::stringstream buffer;
	buffer << t1.rdbuf();
	t1.close();
	std::string szContent(buffer.str());

	char szBuf[1024 * 64] = { 0 };
	sprintf(szBuf, g_szResponse, 200, HttpHelp::mg_get_response_code_text(200), GetTimeHandler()->GetTimeStr(), szContent.size(), szContent.c_str());
	if (!refHttpSession.Send(szBuf, strlen(szBuf)))
	{
		//refHttpSession.Close();
	}
}

class Init
{
public:
	Init()
	{
		CHttpSession::RegistCallBack("/test", HttpCallBackTest);
		CHttpSession::RegistCallBack("/index", HttpCallBackIndex);
		CHttpSession::RegistCallBack("/index.html", HttpCallBackIndex);
	}
	~Init(){}

private:

};

Init oInit;

