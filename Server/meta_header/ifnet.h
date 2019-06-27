#ifndef __IFNET_H__
#define __IFNET_H__

#ifdef WIN32
#include <WinSock2.h>

#else
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#define SOCKET unsigned int
//#define DLLCLASS_DECL
#define INVALID_SOCKET (unsigned int)(-1)
#endif

#include <assert.h>
#include "fxmeta.h"
#include <string.h>

class FxConnection;

class IFxDataHeader;

#define LINUX_NETTHREAD_COUNT		3	// linux默认网络线程数,Windows默认采用cpu个数的2倍 现在win下也是2个
#define MAX_CONNECTION_COUNT		32
#define MAX_NETEVENT_PERSOCK		32
// Max socket count
#define MAX_SOCKET_COUNT			1024*32		// 要比MAX_CONNECTION_COUNT 大


enum ENetErrCode{
	NET_UNKNOWN_ERROR				= -100,
	NET_CLOSE_ERROR					= -9,
	NET_RECVBUFF_ERROR				= -8,
	NET_CONNECT_FAIL				= -7,
	NET_SYSTEM_ERROR				= -6, 
	NET_RECV_ERROR					= -5, 
	NET_SEND_ERROR					= -4, 
	NET_SEND_OVERFLOW				= -3,
	NET_PACKET_ERROR				= -2,
	NET_EOF_ERROR					= -1,
	NET_SUCCESS						= 0
};

enum ESessionOpt
{
	ESESSION_SENDLINGER				= 1,	// 发送延迟，直到成功，或者30次后，默认不打开
};

enum ENetOpt
{
	ENET_MAX_CONNECTION				= 1,	// 最大连接数
	ENET_MAX_TOTALEVENT,					// 每个Socket的最大事件数量
};

enum ESocketState
{
	SSTATE_INVALID = 0,	// //
	//SSTATE_START_LISTEN, // //
	SSTATE_LISTEN,		// //
	SSTATE_STOP_LISTEN,	// //
	//SSTATE_ACCEPT,
	SSTATE_CONNECT,		// //
	SSTATE_ESTABLISH,	// //
	//SSTATE_DATA,		 // //
	SSTATE_CLOSE,		// //
	//SSTATE_OK,
	SSTATE_RELEASE,
};

enum ENetEvtType
{
	NETEVT_INVALID = 0,
	NETEVT_ESTABLISH,
	NETEVT_ASSOCIATE,
	NETEVT_RECV,
	NETEVT_RECV_PACKAGE_ERROR,
	NETEVT_CONN_ERR,
	NETEVT_ERROR,
	NETEVT_TERMINATE,
	NETEVT_RELEASE,
};

enum ESocketType
{
	SLT_None = 0,
	SLT_CommonTcp,
	SLT_WebSocket,
	SLT_Udp,
	SLT_Http,
};

#ifdef WIN32
enum EIocpOperation
{
	IOCP_RECV = 1,
	IOCP_SEND,
	IOCP_ACCEPT,
	IOCP_CONNECT,
};

struct SPerIoData
{
	OVERLAPPED						stOverlapped;
	SOCKET							hSock;
	EIocpOperation					nOp;
	WSABUF							stWsaBuf;
	char							Buf[128];
};

struct SPerUDPIoData
{
	OVERLAPPED						stOverlapped;
	SOCKET							hSock;
	EIocpOperation					nOp;
	WSABUF							stWsaBuf;
	sockaddr_in						stRemoteAddr;
};
#endif // WIN32

struct SNetEvent
{
	ENetEvtType						eType;
	unsigned int					dwValue;
};

class FxSession
{
public:
	FxSession();
	virtual ~FxSession();

	virtual void					OnConnect(void) = 0;

	virtual void					OnClose(void) = 0;

	virtual void					OnError(unsigned int dwErrorNo) = 0;

	virtual void					OnRecv(const char* pBuf, unsigned int dwLen) = 0;

	virtual void					Release(void) = 0;

	virtual char*					GetRecvBuf() = 0;

	virtual unsigned int			GetRecvSize() = 0;

	virtual unsigned int		 	GetRemoteIP();

	virtual const char* 			GetRemoteIPStr();

	virtual unsigned int			GetRemotePort();

	virtual int						GetConnectId();

	virtual bool					Send(const char* pBuf,unsigned int dwLen);

	virtual void					Close(void);

	virtual SOCKET					Reconnect(void);

	virtual bool					IsConnected(void);
	virtual bool					IsConnecting(void);

	virtual void					Init(FxConnection* poConnection);

	virtual bool					SetSessionOpt(ESessionOpt eOpt, bool bSetting);

	//virtual void					SetDataHeader(IFxDataHeader* pDataHeader);

	//virtual IFxDataHeader*			GetDataHeader(){ return m_pDataHeader; }
	virtual IFxDataHeader*			GetDataHeader() = 0;

	virtual bool					OnDestroy();

	virtual FxConnection*			GetConnection(void);
#ifdef WIN32
	void							ForceSend();//udp中才用的到
#endif // WIN32
protected:
	virtual void					Init(){}
private:
	FxConnection*					m_poConnection;

	//IFxDataHeader*					m_pDataHeader;
};

class IFxSessionFactory
{
public:
	virtual							~IFxSessionFactory() {}
	virtual FxSession*				CreateSession() = 0;
	virtual void					Release(FxSession* pSession) = 0;
};

class IFxSocket
{
public:
	virtual							~IFxSocket(){}

	virtual bool					Init() = 0;
	virtual void					OnRead() = 0;
	virtual void					OnWrite() = 0;
	virtual bool					Close() = 0;
	virtual void					ProcEvent(SNetEvent oEvent) = 0;
	
	void							SetSock(SOCKET hSock){ m_hSock = hSock; }
	SOCKET&							GetSock(){ return m_hSock; }
	void							SetSockId(unsigned int dwSockId){ m_dwSockId = dwSockId; }
	unsigned int					GetSockId(){ return m_dwSockId; }

#ifdef WIN32
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, unsigned int dwByteTransferred) = 0;    // 
#else
	virtual void					OnParserIoEvent(int dwEvents) = 0;    // 
#endif // WIN32


protected:
private:
	SOCKET							m_hSock;

	unsigned int					m_dwSockId;
};

class IFxDataHeader
{
public:
	virtual							~IFxDataHeader(){}

	virtual unsigned int			GetHeaderLength() = 0;		// 消息头长度

	virtual int						ParsePacket(const char* pBuf, unsigned int dwLen);

	virtual void*					GetPkgHeader() = 0;			//有歧义了 现在只代表接收到的包头
	virtual void*					BuildSendPkgHeader(unsigned int& dwHeaderLen, unsigned int dwDataLen) = 0;
	virtual bool					BuildRecvPkgHeader(char* pBuff, unsigned int dwLen, unsigned int dwOffset) = 0;
	virtual int						__CheckPkgHeader(const char* pBuf) = 0;

};

class IFxDataHeaderFactory
{
public:
	virtual							~IFxDataHeaderFactory(){}

	virtual IFxDataHeader*			CreateDataHeader() = 0;

private:

};

class IFxListenSocket : public IFxSocket
{
public:
	virtual							~IFxListenSocket(){}

	virtual bool					Init() = 0;

	bool							Init(IFxSessionFactory* pSessionFactory){ m_poSessionFactory = pSessionFactory; return Init(); }

	virtual void					OnRead() = 0;
	virtual void					OnWrite() = 0;
	virtual SOCKET					Listen(unsigned int dwIP, unsigned short& wPort) = 0;
	virtual bool					StopListen() = 0;
	virtual bool					Close() = 0;
	virtual void					ProcEvent(SNetEvent oEvent) = 0;

#ifdef WIN32
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, unsigned int dwByteTransferred) = 0;		//
#else
	virtual void					OnParserIoEvent(int dwEvents) = 0;		//
#endif // WIN32

	IFxSessionFactory*				GetSessionFactory(){ return m_poSessionFactory; }

protected:
	IFxSessionFactory*				m_poSessionFactory;
private:

};

class IFxConnectSocket : public IFxSocket
{
public:
	virtual ~IFxConnectSocket(){}

	virtual bool					Init() = 0;
	//bool							Init(IFxListenSocket* pListenSocket){ m_pListenSocket = pListenSocket;return Init(); }
	virtual void					OnRead() = 0;
	virtual void					OnWrite() = 0;
	virtual bool					Close() = 0;
	virtual void					ProcEvent(SNetEvent oEvent) = 0;
	virtual bool					Send(const char* pData, int dwLen) = 0;

	virtual void					SetConnection(FxConnection* poFxConnection) { m_poConnection = poFxConnection; }
	FxConnection*					GetConnection()								{ return m_poConnection; }
	virtual SOCKET					Connect() = 0;

	virtual bool					IsConnected() = 0;
	virtual IFxDataHeader*			GetDataHeader() = 0;

	virtual bool					PostClose() = 0;
#ifdef WIN32
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, unsigned int dwByteTransferred) = 0;		//
#else
	virtual void					OnParserIoEvent(int dwEvents) = 0;		//
#endif // WIN32

	virtual void					Update() {}

protected:
	FxConnection*					m_poConnection;
private:

};

class IFxNet
{
public:
	virtual							~IFxNet() {}
	virtual bool					Init() = 0;
	virtual bool					Run(unsigned int dwCount) = 0;
	virtual void					Release() = 0;

	virtual SOCKET					TcpConnect(FxSession* poSession, unsigned int dwIP, unsigned short wPort, bool bReconnect = false) = 0;
	virtual IFxListenSocket*		Listen(IFxSessionFactory* pSessionFactory, ESocketType eSocketListenType, unsigned int dwIP, unsigned short& dwPort) = 0;

	virtual SOCKET					UdpConnect(FxSession* poSession, unsigned int dwIP, unsigned short wPort, bool bReconnect = false) = 0;

private:

};

IFxNet* FxNetGetModule();

typedef IFxNet* (*PFN_FxNetGetModule)();

#define HTTP_MAX_HEADERS 64
struct HttpHeader
{
	const char *name;  /* HTTP header name */
	const char *value; /* HTTP header value */
};

struct HttpRequestInfo {
	const char *request_method; /* "GET", "POST", etc */
	const char *request_uri;    /* URL-decoded URI (absolute or relative,
								* as in the request) */
	//const char *local_uri;      /* URL-decoded URI (relative). Can be NULL
	//							* if the request_uri does not address a
	//							* resource at the server host. */
	const char *http_version; /* E.g. "1.0", "1.1" */
	const char *query_string; /* URL part after '?', not including '?', or
							  NULL */
	//const char *remote_user;  /* Authenticated user, or NULL if no auth
	//						  used */
	//char remote_addr[48];     /* Client's IP address as a string. */


	long long content_length; /* Length (in bytes) of the request body,
							  can be -1 if no length was given. */
	//int remote_port;          /* Client's port */
	int is_ssl;               /* 1 if SSL-ed, 0 if not */
	//void *user_data;          /* User data pointer passed to mg_start() */
	//void *conn_data;          /* Connection-specific user data */

	int num_headers; /* Number of HTTP headers */
	struct HttpHeader http_headers[64]; /* Allocate maximum headers */
	
	const char* body;
};

struct HttpMethodInfo {
	const char *name;
	int request_has_body;
	int response_has_body;
	int is_safe;
	int is_idempotent;
	int is_cacheable;
};


/* https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods */
static struct HttpMethodInfo g_sHttpMethods[] = {
	/* HTTP (RFC 2616) */
	{ "GET", 0, 1, 1, 1, 1 },
	{ "POST", 1, 1, 0, 0, 0 },
	{ "PUT", 1, 0, 0, 1, 0 },
	{ "DELETE", 0, 0, 0, 1, 0 },
	{ "HEAD", 0, 0, 1, 1, 1 },
	{ "OPTIONS", 0, 0, 1, 1, 0 },
	{ "CONNECT", 1, 1, 0, 0, 0 },
	/* TRACE method (RFC 2616) is not supported for security reasons */

	/* PATCH method (RFC 5789) */
	{ "PATCH", 1, 0, 0, 0, 0 },
	/* PATCH method only allowed for CGI/Lua/LSP and callbacks. */

	/* WEBDAV (RFC 2518) */
	{ "PROPFIND", 0, 1, 1, 1, 0 },
	/* http://www.webdav.org/specs/rfc4918.html, 9.1:
	* Some PROPFIND results MAY be cached, with care,
	* as there is no cache validation mechanism for
	* most properties. This method is both safe and
	* idempotent (see Section 9.1 of [RFC2616]). */
	{ "MKCOL", 0, 0, 0, 1, 0 },
	/* http://www.webdav.org/specs/rfc4918.html, 9.1:
	* When MKCOL is invoked without a request body,
	* the newly created collection SHOULD have no
	* members. A MKCOL request message may contain
	* a message body. The precise behavior of a MKCOL
	* request when the body is present is undefined,
	* ... ==> We do not support MKCOL with body data.
	* This method is idempotent, but not safe (see
	* Section 9.1 of [RFC2616]). Responses to this
	* method MUST NOT be cached. */

	/* Unsupported WEBDAV Methods: */
	/* PROPPATCH, COPY, MOVE, LOCK, UNLOCK (RFC 2518) */
	/* + 11 methods from RFC 3253 */
	/* ORDERPATCH (RFC 3648) */
	/* ACL (RFC 3744) */
	/* SEARCH (RFC 5323) */
	/* + MicroSoft extensions
	* https://msdn.microsoft.com/en-us/library/aa142917.aspx */

	/* REPORT method (RFC 3253) */
	{ "REPORT", 1, 1, 1, 1, 1 },
	/* REPORT method only allowed for CGI/Lua/LSP and callbacks. */
	/* It was defined for WEBDAV in RFC 3253, Sec. 3.6
	* (https://tools.ietf.org/html/rfc3253#section-3.6), but seems
	* to be useful for REST in case a "GET request with body" is
	* required. */

	{ NULL, 0, 0, 0, 0, 0 }
	/* end of list */
};

namespace HttpHelp
{

	/* Return HTTP header value, or NULL if not found. */
	static const char *
		get_header(const struct HttpHeader *hdr, int num_hdr, const char *name)
	{
		int i;
		for (i = 0; i < num_hdr; i++) {
			if (!strcmp(name, hdr[i].name)) {
				return hdr[i].value;
			}
		}

		return NULL;
	}
	static int
		get_http_header_len(const char *buf, int buflen)
	{
		int i;
		for (i = 0; i < buflen; i++) {
			/* Do an unsigned comparison in some conditions below */
			const unsigned char c = ((const unsigned char *)buf)[i];

			if ((c < 128) && ((char)c != '\r') && ((char)c != '\n')
				&& !isprint(c)) {
				/* abort scan as soon as one malformed character is found */
				return -1;
			}

			if (i < buflen - 1) {
				if ((buf[i] == '\n') && (buf[i + 1] == '\n')) {
					/* Two newline, no carriage return - not standard compliant,
					* but
					* it
					* should be accepted */
					return i + 2;
				}
			}

			if (i < buflen - 3) {
				if ((buf[i] == '\r') && (buf[i + 1] == '\n') && (buf[i + 2] == '\r')
					&& (buf[i + 3] == '\n')) {
					/* Two \r\n - standard compliant */
					return i + 4;
				}
			}
		}

		return 0;
	}
	static int
		skip_to_end_of_word_and_terminate(char **ppw, int eol)
	{
		/* Forward until a space is found - use isgraph here */
		/* See http://www.cplusplus.com/reference/cctype/ */
		while (isgraph(**ppw)) {
			(*ppw)++;
		}

		/* Check end of word */
		if (eol) {
			/* must be a end of line */
			if ((**ppw != '\r') && (**ppw != '\n')) {
				return -1;
			}
		}
		else {
			/* must be a end of a word, but not a line */
			if (**ppw != ' ') {
				return -1;
			}
		}

		/* Terminate and forward to the next word */
		do {
			**ppw = 0;
			(*ppw)++;
		} while ((**ppw) && isspace(**ppw));

		/* Check after term */
		if (!eol) {
			/* if it's not the end of line, there must be a next word */
			if (!isgraph(**ppw)) {
				return -1;
			}
		}

		/* ok */
		return 1;
	}
	static const struct HttpMethodInfo*
		get_http_method_info(const char *method)
	{
		/* Check if the method is known to the server. The list of all known
		* HTTP methods can be found here at
		* http://www.iana.org/assignments/http-methods/http-methods.xhtml
		*/
		const struct HttpMethodInfo *m = g_sHttpMethods;

		while (m->name) {
			if (!strcmp(m->name, method)) {
				return m;
			}
			m++;
		}
		return NULL;
	}
	static int
		is_valid_http_method(const char *method)
	{
		return (get_http_method_info(method) != NULL);
	}
	static int
		parse_http_headers(char **buf, struct HttpHeader hdr[HTTP_MAX_HEADERS])
	{
		int i;
		int num_headers = 0;

		for (i = 0; i < (int)HTTP_MAX_HEADERS; i++) {
			char *dp = *buf;
			while ((*dp != ':') && (*dp >= 33) && (*dp <= 126)) {
				dp++;
			}
			if (dp == *buf) {
				/* End of headers reached. */
				break;
			}
			if (*dp != ':') {
				/* This is not a valid field. */
				return -1;
			}

			/* End of header key (*dp == ':') */
			/* Truncate here and set the key name */
			*dp = 0;
			hdr[i].name = *buf;
			do {
				dp++;
			} while (*dp == ' ');

			/* The rest of the line is the value */
			hdr[i].value = dp;
			*buf = dp + strcspn(dp, "\r\n");
			if (((*buf)[0] != '\r') || ((*buf)[1] != '\n')) {
				*buf = NULL;
			}

			num_headers = i + 1;
			if (*buf) {
				(*buf)[0] = 0;
				(*buf)[1] = 0;
				*buf += 2;
			}
			else {
				*buf = dp;
				break;
			}

			if ((*buf)[0] == '\r') {
				/* This is the end of the header */
				break;
			}
		}
		return num_headers;
	}
	static int
		parse_http_request(char *buf, int len, struct HttpRequestInfo* ri)
	{
		int request_length;
		int init_skip = 0;

		/* Reset attributes. DO NOT TOUCH is_ssl, remote_ip, remote_addr,
		* remote_port */
		ri->request_method = ri->request_uri = ri->http_version =
			NULL;
		ri->num_headers = 0;

		/* RFC says that all initial whitespaces should be ingored */
		/* This included all leading \r and \n (isspace) */
		/* See table: http://www.cplusplus.com/reference/cctype/ */
		while ((len > 0) && isspace(*(unsigned char *)buf)) {
			buf++;
			len--;
			init_skip++;
		}

		if (len == 0) {
			/* Incomplete request */
			return 0;
		}

		/* Control characters are not allowed, including zero */
		if (iscntrl(*(unsigned char *)buf)) {
			return -1;
		}

		/* Find end of HTTP header */
		request_length = get_http_header_len(buf, len);
		if (request_length <= 0) {
			return request_length;
		}
		buf[request_length - 1] = '\0';

		if ((*buf == 0) || (*buf == '\r') || (*buf == '\n')) {
			return -1;
		}

		/* The first word has to be the HTTP method */
		ri->request_method = buf;

		if (skip_to_end_of_word_and_terminate(&buf, 0) <= 0) {
			return -1;
		}

		/* Check for a valid http method */
		if (!is_valid_http_method(ri->request_method)) {
			return -1;
		}

		/* The second word is the URI */
		ri->request_uri = buf;

		if (skip_to_end_of_word_and_terminate(&buf, 0) <= 0) {
			return -1;
		}
		ri->query_string = strchr(ri->request_uri, '?');
		if (ri->query_string)
		{
			if (ri->query_string[0] == '?')
			{
				const_cast<char*>(ri->query_string)[0] = 0;
				ri->query_string += 1;
			}
		}

		/* Next would be the HTTP version */
		ri->http_version = buf;

		if (skip_to_end_of_word_and_terminate(&buf, 1) <= 0) {
			return -1;
		}

		/* Check for a valid HTTP version key */
		if (strncmp(ri->http_version, "HTTP/", 5) != 0) {
			/* Invalid request */
			return -1;
		}
		ri->http_version += 5;

		/* Parse all HTTP headers */
		ri->num_headers = parse_http_headers(&buf, ri->http_headers);
		if (ri->num_headers < 0) {
			/* Error while parsing headers */
			return -1;
		}

		const char *cl;
		if ((cl = get_header(ri->http_headers,
			ri->num_headers,
			"Content-Length")) != NULL) {
			/* Request/response has content length set */
			char *endptr = NULL;
			ri->content_length = strtoll(cl, &endptr, 10);
			if (skip_to_end_of_word_and_terminate(&buf, 1) <= 0) {
				return -1;
			}
			ri->body = buf + 1;
		}

		return request_length + init_skip;
	}
	static const char * mg_get_response_code_text(int response_code)
	{
		/* See IANA HTTP status code assignment:
		* http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
		*/

		switch (response_code) {
			/* RFC2616 Section 10.1 - Informational 1xx */
		case 100:
			return "Continue"; /* RFC2616 Section 10.1.1 */
		case 101:
			return "Switching Protocols"; /* RFC2616 Section 10.1.2 */
		case 102:
			return "Processing"; /* RFC2518 Section 10.1 */

								 /* RFC2616 Section 10.2 - Successful 2xx */
		case 200:
			return "OK"; /* RFC2616 Section 10.2.1 */
		case 201:
			return "Created"; /* RFC2616 Section 10.2.2 */
		case 202:
			return "Accepted"; /* RFC2616 Section 10.2.3 */
		case 203:
			return "Non-Authoritative Information"; /* RFC2616 Section 10.2.4 */
		case 204:
			return "No Content"; /* RFC2616 Section 10.2.5 */
		case 205:
			return "Reset Content"; /* RFC2616 Section 10.2.6 */
		case 206:
			return "Partial Content"; /* RFC2616 Section 10.2.7 */
		case 207:
			return "Multi-Status"; /* RFC2518 Section 10.2, RFC4918 Section 11.1
								   */
		case 208:
			return "Already Reported"; /* RFC5842 Section 7.1 */

		case 226:
			return "IM used"; /* RFC3229 Section 10.4.1 */

							  /* RFC2616 Section 10.3 - Redirection 3xx */
		case 300:
			return "Multiple Choices"; /* RFC2616 Section 10.3.1 */
		case 301:
			return "Moved Permanently"; /* RFC2616 Section 10.3.2 */
		case 302:
			return "Found"; /* RFC2616 Section 10.3.3 */
		case 303:
			return "See Other"; /* RFC2616 Section 10.3.4 */
		case 304:
			return "Not Modified"; /* RFC2616 Section 10.3.5 */
		case 305:
			return "Use Proxy"; /* RFC2616 Section 10.3.6 */
		case 307:
			return "Temporary Redirect"; /* RFC2616 Section 10.3.8 */
		case 308:
			return "Permanent Redirect"; /* RFC7238 Section 3 */

										 /* RFC2616 Section 10.4 - Client Error 4xx */
		case 400:
			return "Bad Request"; /* RFC2616 Section 10.4.1 */
		case 401:
			return "Unauthorized"; /* RFC2616 Section 10.4.2 */
		case 402:
			return "Payment Required"; /* RFC2616 Section 10.4.3 */
		case 403:
			return "Forbidden"; /* RFC2616 Section 10.4.4 */
		case 404:
			return "Not Found"; /* RFC2616 Section 10.4.5 */
		case 405:
			return "Method Not Allowed"; /* RFC2616 Section 10.4.6 */
		case 406:
			return "Not Acceptable"; /* RFC2616 Section 10.4.7 */
		case 407:
			return "Proxy Authentication Required"; /* RFC2616 Section 10.4.8 */
		case 408:
			return "Request Time-out"; /* RFC2616 Section 10.4.9 */
		case 409:
			return "Conflict"; /* RFC2616 Section 10.4.10 */
		case 410:
			return "Gone"; /* RFC2616 Section 10.4.11 */
		case 411:
			return "Length Required"; /* RFC2616 Section 10.4.12 */
		case 412:
			return "Precondition Failed"; /* RFC2616 Section 10.4.13 */
		case 413:
			return "Request Entity Too Large"; /* RFC2616 Section 10.4.14 */
		case 414:
			return "Request-URI Too Large"; /* RFC2616 Section 10.4.15 */
		case 415:
			return "Unsupported Media Type"; /* RFC2616 Section 10.4.16 */
		case 416:
			return "Requested range not satisfiable"; /* RFC2616 Section 10.4.17
													  */
		case 417:
			return "Expectation Failed"; /* RFC2616 Section 10.4.18 */

		case 421:
			return "Misdirected Request"; /* RFC7540 Section 9.1.2 */
		case 422:
			return "Unproccessable entity"; /* RFC2518 Section 10.3, RFC4918
											* Section 11.2 */
		case 423:
			return "Locked"; /* RFC2518 Section 10.4, RFC4918 Section 11.3 */
		case 424:
			return "Failed Dependency"; /* RFC2518 Section 10.5, RFC4918
										* Section 11.4 */

		case 426:
			return "Upgrade Required"; /* RFC 2817 Section 4 */

		case 428:
			return "Precondition Required"; /* RFC 6585, Section 3 */
		case 429:
			return "Too Many Requests"; /* RFC 6585, Section 4 */

		case 431:
			return "Request Header Fields Too Large"; /* RFC 6585, Section 5 */

		case 451:
			return "Unavailable For Legal Reasons"; /* draft-tbray-http-legally-restricted-status-05,
													* Section 3 */

													/* RFC2616 Section 10.5 - Server Error 5xx */
		case 500:
			return "Internal Server Error"; /* RFC2616 Section 10.5.1 */
		case 501:
			return "Not Implemented"; /* RFC2616 Section 10.5.2 */
		case 502:
			return "Bad Gateway"; /* RFC2616 Section 10.5.3 */
		case 503:
			return "Service Unavailable"; /* RFC2616 Section 10.5.4 */
		case 504:
			return "Gateway Time-out"; /* RFC2616 Section 10.5.5 */
		case 505:
			return "HTTP Version not supported"; /* RFC2616 Section 10.5.6 */
		case 506:
			return "Variant Also Negotiates"; /* RFC 2295, Section 8.1 */
		case 507:
			return "Insufficient Storage"; /* RFC2518 Section 10.6, RFC4918
										   * Section 11.5 */
		case 508:
			return "Loop Detected"; /* RFC5842 Section 7.1 */

		case 510:
			return "Not Extended"; /* RFC 2774, Section 7 */
		case 511:
			return "Network Authentication Required"; /* RFC 6585, Section 6 */

													  /* Other status codes, not shown in the IANA HTTP status code
													  * assignment.
													  * E.g., "de facto" standards due to common use, ... */
		case 418:
			return "I am a teapot"; /* RFC2324 Section 2.3.2 */
		case 419:
			return "Authentication Timeout"; /* common use */
		case 420:
			return "Enhance Your Calm"; /* common use */
		case 440:
			return "Login Timeout"; /* common use */
		case 509:
			return "Bandwidth Limit Exceeded"; /* common use */

		default:
			/* Return at least a category according to RFC 2616 Section 10. */
			if (response_code >= 100 && response_code < 200) {
				/* Unknown informational status code */
				return "Information";
			}
			if (response_code >= 200 && response_code < 300) {
				/* Unknown success code */
				return "Success";
			}
			if (response_code >= 300 && response_code < 400) {
				/* Unknown redirection code */
				return "Redirection";
			}
			if (response_code >= 400 && response_code < 500) {
				/* Unknown request error code */
				return "Client Error";
			}
			if (response_code >= 500 && response_code < 600) {
				/* Unknown server error code */
				return "Server Error";
			}

			/* Response code not even within reasonable range */
			return "";
		}
	}
};


#endif	// __IFNET_H__

