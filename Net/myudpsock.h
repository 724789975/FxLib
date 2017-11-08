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
		return end - begin;
	}

	// is valid id
	inline bool IsValidIndex(unsigned char id)
	{
		unsigned char pos = id - begin;
		unsigned char count = end - begin;
		return pos < count;
	}

	// clear
	inline void ClearBuffer()
	{
		// link buffers
		free_buffer_id = 0;
		for (int i = 0; i < window_size; i++)
			buffer[i][0] = i + 1;
	}

public:
	static const unsigned int buffer_size = 512;
	static const unsigned int window_size = 32;

	// temp buffer
	unsigned char buffer[window_size][buffer_size];
	unsigned char free_buffer_id;

	unsigned char seq_buffer_id[window_size];
	unsigned short seq_size[window_size];
	double seq_time[window_size];
	double seq_retry[window_size];
	double seq_retry_time[window_size];
	unsigned int seq_retry_count[window_size];

	unsigned char begin;	// begin position of sliding window
	unsigned char end;	// end position of sliding window
};

struct UDPPacketHeader
{
	char m_cStatus;
	char m_cSyn;
	char m_cAck;
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

	virtual void ProcEvent(SNetEvent oEvent);

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
	FxIoThread* m_poIoThreadHandler;
	FxLoopBuff*         m_poRecvBuf;
#ifdef WIN32
	SPerUDPIoData m_oSPerIoDatas[128];
	UDPPacketHeader m_oPacketHeaders[128];
#endif // WIN32




	// bytes send and recieved
	unsigned int num_bytes_send;
	unsigned int num_bytes_received;

	// network delay time.
	double delay_time;
	double delay_average;
	double retry_time;
	double send_time;
	double send_frequency;
	double send_window_control;
	double send_window_threshhold;
	double send_data_time;
	double send_data_frequency;

	SlidingWindow recv_window;
	SlidingWindow send_window;

	// packets count send and retry
	unsigned int num_packets_send;
	unsigned int num_packets_retry;


private:
	double ack_recv_time;
	int ack_timeout_retry;
	//unsigned int status;

	int ack_same_count;
	bool quick_retry;
	bool send_ack;
	unsigned char ack_last;
	unsigned char syn_last;
};

class FxConnection;
class FxUDPConnectSock : public IFxConnectSocket
{
	friend class FxUDPListenSock;
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

	virtual void ProcEvent(SNetEvent oEvent);

	SOCKET Connect();

	void SetRemoteAddr(sockaddr_in& refstRemoteAddr);
	virtual bool PostClose();
#ifdef WIN32
	bool PostRecv();
	bool PostRecvFree();

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




	// bytes send and recieved
	unsigned int num_bytes_send;
	unsigned int num_bytes_received;

	// network delay time.
	double delay_time;
	double delay_average;
	double retry_time;
	double send_time;
	double send_frequency;
	double send_window_control;
	double send_window_threshhold;
	double send_data_time;
	double send_data_frequency;

	SlidingWindow recv_window;
	SlidingWindow send_window;

	// packets count send and retry
	unsigned int num_packets_send;
	unsigned int num_packets_retry;


private:
	double ack_recv_time;
	int ack_timeout_retry;
	unsigned int status;

	int ack_same_count;
	bool quick_retry;
	bool send_ack;
	unsigned char ack_last;
	unsigned char syn_last;

};


#endif // !__MyUdpSock_h__
