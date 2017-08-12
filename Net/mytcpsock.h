#ifndef __MyTcpSock_h__
#define __MyTcpSock_h__

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <mswsock.h>
#endif

#include "fxmeta.h"
#include "eventqueue.h"
#include "ifnet.h"
#include "loopbuff.h"
#include "lock.h"

class FxIoThread;

class FxTCPListenSock : public IFxListenSocket
{
public:
	FxTCPListenSock();
	virtual ~FxTCPListenSock();

	virtual bool						Init();
	virtual void						OnRead();
	virtual void						OnWrite();
	virtual bool						Listen(UINT32 dwIP, UINT16 wPort);
	virtual bool						StopListen();
	virtual bool						Close();
	void								Reset();

	bool								PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	void								SetState(ESocketState eState)					{ m_nState = eState; }
	ESocketState						GetState()										{ return m_nState; }

	virtual void ProcEvent(SNetEvent oEvent);

#ifdef WIN32
	virtual void						OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		//
#else
	virtual void						OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

public:

protected:
	bool AddEvent();

	void								__ProcAssociate();
	void								__ProcError(UINT32 dwErrorNo);
	void								__ProcTerminate();

protected:
	ESocketState		m_nState;

	FxCriticalLock			m_oLock;

	FxIoThread* m_poIoThreadHandler;

#ifdef WIN32
	bool								PostAccept(SPerIoData& oSPerIoData);
	bool								InitAcceptEx();
	virtual void OnAccept(SPerIoData* pstPerIoData);

	SPerIoData							m_oSPerIoDatas[128];
	LPFN_ACCEPTEX						m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS			m_lpfnGetAcceptExSockaddrs;
#else
	virtual  void OnAccept();
#endif // WIN32
};

class FxWebSocketListen : public FxTCPListenSock
{
public:
	FxWebSocketListen();
	~FxWebSocketListen();

private:
#ifdef WIN32
	virtual void OnAccept(SPerIoData* pstPerIoData);
#else
	virtual  void OnAccept();
#endif // WIN32
};

class FxConnection;
class FxTCPConnectSockBase : public IFxConnectSocket
{
public:
	FxTCPConnectSockBase();
	virtual ~FxTCPConnectSockBase();

	virtual bool						Init();
	virtual void						OnRead();
	virtual void						OnWrite();

	virtual void						Reset();

	virtual bool						Close();
	
	virtual bool						Send(const char* pData, int dwLen);

	bool								PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	bool								IsConnected()									{ return m_nState == SSTATE_ESTABLISH; }
	void								SetState(ESocketState eState)					{ m_nState = eState; }
	ESocketState						GetState()										{ return m_nState; }

	virtual IFxDataHeader*				GetDataHeader();

	void								SetIoThread(FxIoThread* pIoThread)				{ m_poIoThreadHandler = pIoThread;}
	bool								AddEvent();

	virtual void						ProcEvent(SNetEvent oEvent);

	SOCKET								Connect();

	virtual bool						PostClose();
#ifdef WIN32
	bool								PostRecv();
	bool								PostRecvFree();

	virtual void						OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		// 处理完成端口事件//
#else
	virtual void						OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

protected:

	bool								PostSend();
	bool								PostSendFree();
	bool								SendImmediately();						// //

	void								__ProcEstablish();
	void								__ProcAssociate();
	void								__ProcConnectError(UINT32 dwErrorNo);
	void								__ProcError(UINT32 dwErrorNo);
	void								__ProcTerminate();
	virtual void						__ProcRecv(UINT32 dwLen) = 0;
	virtual void						__ProcRelease() = 0;
protected:
	void								OnConnect();
#ifdef WIN32
	virtual void						OnRecv(bool bRet, int dwBytes);
	void								OnSend(bool bRet, int dwBytes);
#else
	virtual void						OnRecv();
	void								OnSend();
#endif // WIN32

protected:
	ESocketState						m_nState;

	bool					m_bSendLinger;		// 发送延迟，直到成功，或者30次后 //

	FxCriticalLock						m_oLock;

	FxLoopBuff*							m_poSendBuf;
	FxLoopBuff*							m_poRecvBuf;

	FxIoThread*							m_poIoThreadHandler;

	int									m_nNeedData;        // 未处理完的逻辑数据包还需要的长度//
	int									m_nPacketLen;       // 未处理完的逻辑数据包长度//

protected:
#ifdef WIN32
	SPerIoData							m_stRecvIoData;
	SPerIoData							m_stSendIoData;
	LONG            				    m_nPostRecv;        // 未决的WSARecv操作数//
	LONG            				    m_nPostSend;        // 未决的WSASend操作数/

	UINT32          				    m_dwLastError;      // 最后的出错信息//
#else
	bool								m_bSending;
#endif // WIN32

};

class FxTCPConnectSock : public FxTCPConnectSockBase
{
public:
	FxTCPConnectSock();
	virtual ~FxTCPConnectSock();

private:
	void __ProcRecv(UINT32 dwLen);
	void __ProcRelease();
};

class FxWebSocketConnect : public FxTCPConnectSockBase
{
public:
	enum EWebSocketHandShakeState
	{
		WSHSS_None = 0,
		WSHSS_Request,
		WSHSS_Response,
		WSHSS_Connected,
	};
public:
	FxWebSocketConnect();
	virtual ~FxWebSocketConnect();

	virtual void Reset();
private:
	void __ProcRecv(UINT32 dwLen);
	void __ProcRelease();

protected:
#ifdef WIN32
	virtual void OnRecv(bool bRet, int dwBytes);
#else
	virtual void OnRecv();
#endif // WIN32

private:
	EWebSocketHandShakeState m_eWebSocketHandShakeState;
	char m_szWebInfo[1024];
};




#endif // !__MySock_h__
