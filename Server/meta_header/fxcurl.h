
#ifndef __FXCURL_H__
#define __FXCURL_H__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <WinSock2.h>
#include <windows.h>
#else
#include <string.h>
#include <stdarg.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h> 
#include <unistd.h> 
#include <signal.h> 
#include <sys/time.h>
#include <errno.h>
#include <arpa/inet.h>
#endif

#include "../curl/curl.h"
#include <assert.h>
#include <string>

#include "fxmeta.h"

class IUrlRequest 
{
public:
    virtual   ~IUrlRequest(){}
	
	virtual void        OnRequest() = 0;
	
    virtual void        OnResult(void) = 0;
	
    virtual void        Release(void) = 0;

protected:
	CURL *m_pCurl;
	CURLcode m_oCode;
	std::string m_szBuffer;
};

class IUrlRequestModule
{
public:
    virtual   ~IUrlRequestModule(){}

	virtual bool        Init() = 0;
	
	virtual void        Close() = 0;
    	
	virtual bool        AddRequest(IUrlRequest *poRequest) = 0;
	
	virtual bool        Run(UINT32 dwCount = -1) = 0;
};

IUrlRequestModule *             FxUrlRequestGetModule();

#endif  // __FXCURL_H__
