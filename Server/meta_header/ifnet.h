#ifndef __IFNET_H__
#define __IFNET_H__

#ifdef WIN32
#include <WinSock2.h>

#else
#include <arpa/inet.h>
#define SOCKET UINT32
//#define DLLCLASS_DECL
#define INVALID_SOCKET UINT32(-1)
#endif

#include <assert.h>
#include "fxmeta.h"

class FxConnection;

class IFxDataHeader;

#define LINUX_NETTHREAD_COUNT		3	// linux默认网络线程数,Windows默认采用cpu个数的2倍 现在win下也是2个
#define MAX_CONNECTION_COUNT		32
#define MAX_NETEVENT_PERSOCK		32
// Max socket count
#define MAX_SOCKET_COUNT			1024*32		// 要比MAX_CONNECTION_COUNT 大


enum ENetErrCode{
	NET_RECVBUFF_ERROR				= -7,
	NET_CONNECT_FAIL				= -6,
	NET_SYSTEM_ERROR				= -5, 
	NET_RECV_ERROR					= -4, 
	NET_SEND_ERROR					= -3, 
	NET_SEND_OVERFLOW				= -2,
	NET_PACKET_ERROR				= -1,
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
	UINT32							dwValue;
};

class FxSession
{
public:
	FxSession();
	virtual ~FxSession();

	virtual void					OnConnect(void) = 0;

	virtual void					OnClose(void) = 0;

	virtual void					OnError(UINT32 dwErrorNo) = 0;

	virtual void					OnRecv(const char* pBuf, UINT32 dwLen) = 0;

	virtual void					Release(void) = 0;

	virtual char*					GetRecvBuf() = 0;

	virtual UINT32					GetRecvSize() = 0;

	virtual UINT32		 			GetRemoteIP();

	virtual const char* 			GetRemoteIPStr();

	virtual UINT32					GetRemotePort();

	virtual bool					Send(const char* pBuf,UINT32 dwLen);

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
private:
	FxConnection*					m_poConnection;

	//IFxDataHeader*					m_pDataHeader;
};

class IFxSessionFactory
{
public:
	virtual							~IFxSessionFactory() {}
	virtual FxSession*				CreateSession() = 0;
	virtual void					Release(FxSession* pSession){}
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
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred) = 0;    // 
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

	virtual int						ParsePacket(const char* pBuf, UINT32 dwLen);

	virtual void*					GetPkgHeader() = 0;			//有歧义了 现在只代表接收到的包头
	virtual void*					BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen) = 0;
	virtual bool					BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset) = 0;
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
	virtual SOCKET					Listen(UINT32 dwIP, UINT16& wPort) = 0;
	virtual bool					StopListen() = 0;
	virtual bool					Close() = 0;
	virtual void					ProcEvent(SNetEvent oEvent) = 0;

#ifdef WIN32
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred) = 0;		//
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
	virtual void					OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred) = 0;		//
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
	virtual bool					Run(UINT32 dwCount) = 0;
	virtual void					Release() = 0;

	virtual SOCKET					TcpConnect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect = false) = 0;
	virtual IFxListenSocket*		Listen(IFxSessionFactory* pSessionFactory, ESocketType eSocketListenType, UINT32 dwIP, UINT16& dwPort) = 0;

	virtual SOCKET					UdpConnect(FxSession* poSession, UINT32 dwIP, UINT16 wPort, bool bReconnect = false) = 0;

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
	const char *local_uri;      /* URL-decoded URI (relative). Can be NULL
								* if the request_uri does not address a
								* resource at the server host. */
	const char *http_version; /* E.g. "1.0", "1.1" */
	const char *query_string; /* URL part after '?', not including '?', or
							  NULL */
	const char *remote_user;  /* Authenticated user, or NULL if no auth
							  used */
	char remote_addr[48];     /* Client's IP address as a string. */


	long long content_length; /* Length (in bytes) of the request body,
							  can be -1 if no length was given. */
	int remote_port;          /* Client's port */
	int is_ssl;               /* 1 if SSL-ed, 0 if not */
	void *user_data;          /* User data pointer passed to mg_start() */
	void *conn_data;          /* Connection-specific user data */

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


#endif	// __IFNET_H__

