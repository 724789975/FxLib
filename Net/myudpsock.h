#ifndef __MyUdpSock_h__
#define __MyUdpSock_h__

#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <mswsock.h>
#else
#include <errno.h>
#endif

#include "fxmeta.h"
#include "eventqueue.h"
#include "ifnet.h"
#include "loopbuff.h"
#include "lock.h"

class FxIoThread;

struct UDPPacketHeader
{
	char m_cStatus;
	char m_cSyn;
	char m_cAck;
	char m_cParam;						// 占位
};

class FxUDPListenSock : public IFxListenSocket
{
public:
	FxUDPListenSock();
	virtual ~FxUDPListenSock();

	virtual bool Init();
	virtual void OnRead();
	virtual void OnWrite();
	virtual bool Listen(UINT32 dwIP, UINT16 wPort);
	virtual bool StopListen();
	virtual bool Close();
	void Reset();

	bool PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	void SetState(ESocketState eState) { m_nState = eState; }
	ESocketState GetState() { return m_nState; }

	virtual void ProcEvent();

#ifdef WIN32
	virtual void OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		//
#else
	virtual void OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

private:
	bool AddEvent();

	void	__ProcAssociate();
	void	__ProcError(UINT32 dwErrorNo);
	void	__ProcTerminate();

#ifdef WIN32
	bool PostAccept(SPerUDPIoData& oSPerIoData);
	void OnAccept(SPerUDPIoData* pstPerIoData);
#else
	void OnAccept();
#endif // WIN32

private:
	ESocketState		m_nState;
	FxCriticalLock			m_oLock;
	TEventQueue<SNetEvent>	m_oEvtQueue;
	FxIoThread* m_poIoThreadHandler;
	FxLoopBuff*         m_poRecvBuf;
#ifdef WIN32
	SPerUDPIoData m_oSPerIoDatas[128];
	UDPPacketHeader m_oPacketHeaders[128];
#endif // WIN32
};

class FxConnection;
class FxUDPConnectSock : public IFxConnectSocket
{
public:
	FxUDPConnectSock();
	virtual ~FxUDPConnectSock();

	virtual bool Init();
	virtual void OnRead();
	virtual void OnWrite();

	void Reset();

	virtual bool Close();

	virtual bool Send(const char* pData, int dwLen);

	bool PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	bool IsConnected() { return m_nState == SSTATE_ESTABLISH; }
	void SetState(ESocketState eState) { m_nState = eState; }
	ESocketState GetState() { return m_nState; }

	virtual IFxDataHeader* GetDataHeader();

	void SetIoThread(FxIoThread* pIoThread) { m_poIoThreadHandler = pIoThread; }
	bool AddEvent();

	virtual void ProcEvent();

	SOCKET Connect();

	void SetRemoteAddr(sockaddr_in& refstRemoteAddr);
#ifdef WIN32
	bool PostRecv();
	bool PostRecvFree();
	virtual bool PostClose();

	virtual void OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		// 处理完成端口事件//
#else
	virtual void OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

private:

	bool PostSend();
	bool PostSendFree();
	bool SendImmediately();						// //

	void	__ProcEstablish();
	void	__ProcAssociate();
	void	__ProcConnectError(UINT32 dwErrorNo);
	void	__ProcError(UINT32 dwErrorNo);
	void	__ProcTerminate();
	void	__ProcRecv(UINT32 dwLen);
	void	__ProcRecvPackageError(UINT32 dwLen);
	void	__ProcRelease();
private:
	void OnConnect();

	bool IsValidAck(char cAck);
#ifdef WIN32
	void OnRecv(bool bRet, int dwBytes);
	void OnSend(bool bRet, int dwBytes);
#else
	void OnRecv();
	void OnSend();
#endif // WIN32

private:
	ESocketState		m_nState;

	TEventQueue<SNetEvent>	m_oEvtQueue;
	bool					m_bSendLinger;		// 发送延迟，直到成功，或者30次后 //

	FxCriticalLock			m_oLock;

	FxLoopBuff*         m_poSendBuf;
	FxLoopBuff*         m_poRecvBuf;

	FxIoThread* m_poIoThreadHandler;

	int m_nNeedData;        // 未处理完的逻辑数据包还需要的长度//
	int m_nPacketLen;       // 未处理完的逻辑数据包长度//

	char m_cSyn;			// 
	char m_cAck;			//

	char m_cDelay;			// 延迟接收次数

	sockaddr m_stRemoteAddr;

private:
#ifdef WIN32
	SPerUDPIoData			m_stRecvIoData;
	SPerUDPIoData			m_stSendIoData;
	LONG                m_nPostRecv;        // 未决的WSARecv操作数//
	LONG                m_nPostSend;        // 未决的WSASend操作数/

	UINT32              m_dwLastError;      // 最后的出错信息//
#else
	bool				m_bSending;
#endif // WIN32

};


#endif // !__MyUdpSock_h__
