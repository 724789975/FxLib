﻿#ifndef __MySock_h__
#define __MySock_h__

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
//#include "packetparser.h"

enum ESocketState
{
	SSTATE_INVALID = 0,	// ��Ч//
	//SSTATE_START_LISTEN, // ����ʼ����״̬//
	SSTATE_LISTEN,		// ����״̬//
	SSTATE_STOP_LISTEN,	// ����ֹͣ����״̬//
	//SSTATE_ACCEPT,
	SSTATE_CONNECT,		// ��������״̬//
	SSTATE_ESTABLISH,	// ���ӽ���״̬//
	//SSTATE_DATA,		 // ��ݷ���״̬//
	SSTATE_CLOSE,		// �ر�����״̬//
	//SSTATE_OK,
	SSTATE_RELEASE,
};

enum ENetEvtType
{
	NETEVT_INVALID = 0,
	NETEVT_ESTABLISH,
	NETEVT_ASSOCIATE,
	NETEVT_RECV,
	NETEVT_CONN_ERR,
	NETEVT_ERROR,
	NETEVT_TERMINATE,
	NETEVT_RELEASE,
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
	OVERLAPPED		stOverlapped;
	SOCKET			hSock;
	EIocpOperation	nOp;
	WSABUF			stWsaBuf;
	char			Buf[128];
};
#endif // WIN32

struct SNetEvent
{
	ENetEvtType		eType;
	UINT32			dwValue;
};

class FxIoThread;

class FxListenSock : public IFxListenSocket
{
public:
	FxListenSock();
	virtual ~FxListenSock();

	virtual bool Init();
	virtual void OnRead();
	virtual void OnWrite();
	virtual bool Listen(UINT32 dwIP, UINT16 wPort);
	virtual bool StopListen();
	virtual bool Close();
	void Reset();

	bool PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	void SetState(ESocketState eState){ m_nState = eState; }
	ESocketState GetState(){ return m_nState; }

	virtual void ProcEvent();

#ifdef WIN32
	virtual void OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred);		//
#else
	virtual void OnParserIoEvent(int dwEvents);		// ������ɶ˿��¼�//
#endif // WIN32

public:

private:
	bool AddEvent();

	void	__ProcAssociate();
	void	__ProcError(UINT32 dwErrorNo);
	void	__ProcTerminate();

private:
	ESocketState		m_nState;

	FxCriticalLock			m_oLock;

	TEventQueue<SNetEvent>	m_oEvtQueue;

	FxIoThread* m_poIoThreadHandler;

#ifdef WIN32
	bool PostAccept(SPerIoData& oSPerIoData);
	bool InitAcceptEx();
	void OnAccept(SPerIoData* pstPerIoData);

	SPerIoData m_oSPerIoDatas[128];
	LPFN_ACCEPTEX       m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
#else
	void OnAccept();
#endif // WIN32
};

class FxConnection;
class FxConnectSock : public IFxConnectSocket
{
public:
	FxConnectSock();
	virtual ~FxConnectSock();

	virtual bool Init();
	virtual void OnRead();
	virtual void OnWrite();

	void Reset();

	virtual bool Close();
	
	virtual bool Send(const char* pData, int dwLen);

	bool PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	void SetConnection(FxConnection* poConnection){ m_poConnection = poConnection; }
	FxConnection* GetConnection(){ return m_poConnection; }

	bool IsConnected(){ return m_nState == SSTATE_ESTABLISH; }
	void SetState(ESocketState eState){ m_nState = eState; }
	ESocketState GetState(){ return m_nState; }

	IFxDataHeader* GetDataHeader();

	void SetIoThread(FxIoThread* pIoThread){ m_poIoThreadHandler = pIoThread;}
	bool AddEvent();

	virtual void ProcEvent();

	SOCKET Connect();

#ifdef WIN32
	bool PostRecv();
	bool PostClose();
	bool PostRecvFree();

	virtual void OnParserIoEvent(bool bRet, SPerIoData* pIoData, UINT32 dwByteTransferred);		//
#else
	virtual void OnParserIoEvent(int dwEvents);		// ������ɶ˿��¼�//
#endif // WIN32

private:

	bool PostSend();
	bool PostSendFree();
	bool SendImmediately();						// ��Ҫ�����������ȫ������//

	void	__ProcEstablish();
	void	__ProcAssociate();
	void	__ProcConnectError(UINT32 dwErrorNo);
	void	__ProcError(UINT32 dwErrorNo);
	void	__ProcTerminate();
	void	__ProcRecv(UINT32 dwLen);
	void	__ProcRelease();
private:
	void OnConnect();
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
	bool					m_bSendLinger;		// �����ӳ٣�ֱ���ɹ�������30�κ�//

	FxCriticalLock			m_oLock;

	FxLoopBuff*         m_poSendBuf;
	FxLoopBuff*         m_poRecvBuf;
	FxConnection*		m_poConnection;

	FxIoThread* m_poIoThreadHandler;

	int m_nNeedData;        // δ��������߼���ݰ���Ҫ�ĳ���//
	int m_nPacketLen;       // δ��������߼���ݰ��//

private:
#ifdef WIN32
	SPerIoData			m_stRecvIoData;
	SPerIoData			m_stSendIoData;
	LONG                m_nPostRecv;        // δ����WSARecv������//
	LONG                m_nPostSend;        // δ����WSASend������//

	UINT32              m_dwLastError;      // ���ĳ�����Ϣ//
#else
	bool				m_bSending;
#endif // WIN32

};


#endif // !__MySock_h__
