#include "fxtimer.h"
#include "fxcurl.h"
#include "fxmeta.h"

#include "exception_dump.h"

#include <signal.h>

#include <string>

/*
ptr是指向存储数据的指针，
size是每个块的大小，
nmemb是指块的数目，
stream是用户参数。
所以根据以上这些参数的信息可以知道，ptr中的数据的总长度是size*nmemb
*/
size_t call_wirte_func(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}

class TestRequest : public IUrlRequest
{
public:
	TestRequest(){}
	~TestRequest(){}

	virtual void        OnRequest()
	{
		std::string url = "http://192.168.101.97:8090";
		std::string useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1";
		m_pCurl = curl_easy_init();
		if (m_pCurl)
		{
			curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);   //不验证证书和HOST
			curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
			// 远程URL，支持 http, https, ftp
			curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, useragent.c_str());
			// 官方下载的DLL并不支持GZIP，Accept-Encoding:deflate, gzip
			//curl_easy_setopt(m_pCurl, CURLOPT_ENCODING, "gzip, deflate");

			//设置重定向的最大次数
			curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 5);
			//设置301、302跳转跟随location
			curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);

			//抓取内容后，回调函数
			curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, call_wirte_func);
			curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &m_szBuffer);

			m_oCode = curl_easy_perform(m_pCurl);
		}
	}

	virtual void        OnResult(void)
	{
		m_szBuffer.resize(1024);
		LogExe(LogLv_Info, "%s", m_szBuffer.c_str());
	}

	virtual void        Release(void) { delete this; }
private:

};

void DumpTest()
{
	int * p = NULL;
	*p = 12;

	int a1 = 1;
	int a2 = 1;

	float a = 1 / (a1 - a2);
}

int main()
{
	ExceptionDump::RegExceptionHandler();
	IUrlRequestModule* pModule = FxUrlRequestGetModule();
	pModule->Init();

	if (!GetTimeHandler()->Init())
	{
		return 0;
	}
	GetTimeHandler()->Run();

	TestRequest* pTest = new TestRequest;
	pModule->AddRequest(pTest);

	try
	{
		DumpTest();
	}
	catch (const std::exception& e)
	{
		LogExe(LogLv_Error, "%s", e.what());
	}
	catch(...) { }

	while (true)
	{
		GetTimeHandler()->Run();
		pModule->Run();
		FxSleep(10);
	}
}