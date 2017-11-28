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

// sliding window uses in reliable udp transfer.
class SlidingWindow
{
public:
	// count
	inline unsigned char Count()
	{
		return m_btEnd - m_btBegin;
	}

	// is valid id
	inline bool IsValidIndex(unsigned char btId)
	{
		unsigned char btPos = btId - m_btBegin;
		unsigned char btCount = m_btEnd - m_btBegin;
		return btPos < btCount;
	}

	// clear
	inline void ClearBuffer()
	{
		// link buffers
		m_btFreeBufferId = 0;
		for (int i = 0; i < s_dwWindowSize; i++)
			m_ppBuffer[i][0] = i + 1;
	}

public:
	static const unsigned int	s_dwBufferSize = 512;
	static const unsigned int	s_dwWindowSize = 32;

	// temp buffer
	unsigned char				m_ppBuffer[s_dwWindowSize][s_dwBufferSize];
	unsigned char				m_btFreeBufferId;

	unsigned char				m_pSeqBufferId[s_dwWindowSize];
	unsigned short				m_pSeqSize[s_dwWindowSize];
	double						m_pSeqTime[s_dwWindowSize];
	double						m_pSeqRetry[s_dwWindowSize];
	double						m_pSeqRetryTime[s_dwWindowSize];
	unsigned int				m_pSeqRetryCount[s_dwWindowSize];

	unsigned char				m_btBegin;	// begin position of sliding window
	unsigned char				m_btEnd;	// end position of sliding window
};

struct UDPPacketHeader
{
	UDPPacketHeader() { m_cStatus = 255; m_cSyn = 255; m_cAck = 255; }
	unsigned char m_cStatus;
	unsigned char m_cSyn;
	unsigned char m_cAck;
};

class FxUDPListenSock : public IFxListenSocket
{
public:
	FxUDPListenSock();
	virtual ~FxUDPListenSock();

	virtual bool				Init();
	virtual void				OnRead();
	virtual void				OnWrite();
	virtual bool				Listen(UINT32 dwIP, UINT16 wPort);
	virtual bool				StopListen();
	virtual bool				Close();
	void						Reset();

	bool						PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	void						SetState(ESocketState eState) { m_nState = eState; }
	ESocketState				GetState() { return m_nState; }

	virtual void				ProcEvent(SNetEvent oEvent);

#ifdef WIN32
	virtual void				OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		//
#else
	virtual void				OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

private:
	bool						AddEvent();

	void						__ProcAssociate();
	void						__ProcError(UINT32 dwErrorNo);
	void						__ProcTerminate();

#ifdef WIN32
	bool						PostAccept(SPerUDPIoData& oSPerIoData);
	void						OnAccept(SPerUDPIoData* pstPerIoData);
#else
	void						OnAccept();
#endif // WIN32

private:
	ESocketState				m_nState;
	FxCriticalLock				m_oLock;
	FxIoThread*					m_poIoThreadHandler;
	FxLoopBuff*					m_poRecvBuf;
#ifdef WIN32
	SPerUDPIoData				m_oSPerIoDatas[128];
	UDPPacketHeader				m_oPacketHeaders[128];
#endif // WIN32

};

class FxConnection;
class FxUDPConnectSock : public IFxConnectSocket
{
	friend class FxUDPListenSock;
public:
	FxUDPConnectSock();
	virtual ~FxUDPConnectSock();

	virtual bool				Init();
	virtual void				OnRead();
	virtual void				OnWrite();

	void						Reset();

	virtual bool				Close();

	virtual bool				Send(const char* pData, int dwLen);

	bool						PushNetEvent(ENetEvtType eType, UINT32 dwValue);

	bool						IsConnected() { return m_nState == SSTATE_ESTABLISH; }
	void						SetState(ESocketState eState) { m_nState = eState; }
	ESocketState				GetState() { return m_nState; }

	virtual IFxDataHeader*		GetDataHeader();

	void						SetIoThread(FxIoThread* pIoThread) { m_poIoThreadHandler = pIoThread; }
	bool						AddEvent();

	virtual void				ProcEvent(SNetEvent oEvent);

	SOCKET						Connect();

	void						SetRemoteAddr(sockaddr_in& refstRemoteAddr);
	virtual bool				PostClose();
#ifdef WIN32
	bool						PostRecv();
	bool						PostRecvFree();

	virtual void				OnParserIoEvent(bool bRet, void* pIoData, UINT32 dwByteTransferred);		// 处理完成端口事件//
#else
	virtual void				OnParserIoEvent(int dwEvents);		//  1/4 //
#endif // WIN32

private:

	bool						PostSend();
	bool						PostSendFree();
	bool						SendImmediately();						// //

	void						__ProcEstablish();
	void						__ProcAssociate();
	void						__ProcConnectError(UINT32 dwErrorNo);
	void						__ProcError(UINT32 dwErrorNo);
	void						__ProcTerminate();
	void						__ProcRecv(UINT32 dwLen);
	void						__ProcRecvPackageError(UINT32 dwLen);
	void						__ProcRelease();
private:
#ifdef WIN32
	void						OnRecv(bool bRet, int dwBytes);
	void						OnSend(bool bRet, int dwBytes);
#else
	void						OnRecv();
	void						OnSend();
#endif // WIN32

private:
	ESocketState				m_nState;

	bool						m_bSendLinger;		// 发送延迟，直到成功，或者30次后 //

	FxCriticalLock				m_oLock;

	FxLoopBuff*					m_poSendBuf;
	FxLoopBuff*					m_poRecvBuf;

	FxIoThread*					m_poIoThreadHandler;

	int							m_nNeedData;        // 未处理完的逻辑数据包还需要的长度//
	int							m_nPacketLen;       // 未处理完的逻辑数据包长度//

	char						m_cSyn;			// 
	char						m_cAck;			//

	char						m_cDelay;			// 延迟接收次数

	sockaddr					m_stRemoteAddr;

private:
#ifdef WIN32
	SPerUDPIoData				m_stRecvIoData;
	SPerUDPIoData				m_stSendIoData;
	LONG						m_nPostRecv;        // 未决的WSARecv操作数//
	LONG						m_nPostSend;        // 未决的WSASend操作数/

	UINT32						m_dwLastError;      // 最后的出错信息//
#else
#endif // WIN32

	// bytes send and recieved
	unsigned int				num_bytes_send;
	unsigned int				num_bytes_received;

	// network delay time.
	double						m_dDelayTime;
	double						m_dDelayAverage;
	double						m_dRetryTime;
	double						m_dSendTime;
	double						m_dSendFrequency;
	double						m_dSendWindowControl;
	double						m_dSendWindowThreshhold;
	double						m_dSendDataTime;
	double						m_dSendDataFrequency;

	SlidingWindow				m_oRecvWindow;
	SlidingWindow				m_oSendWindow;

	// packets count send and retry
	unsigned int				num_packets_send;
	unsigned int				num_packets_retry;

private:
	double						m_dAckRecvTime;
	int							m_dAckTimeoutRetry;

	int							m_dwAckSameCount;
	bool						m_bQuickRetry;
	bool						m_bSendAck;
	unsigned char				m_btAckLast;
	unsigned char				m_btSynLast;

#ifdef WIN32
	unsigned char				m_byRecvBufferId;
#endif // WIN32
};


#endif // !__MyUdpSock_h__
