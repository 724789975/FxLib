#ifndef __SocketSession_H__
#define __SocketSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>

class CSocketSession : public FxSession
{
public:
	CSocketSession();
	virtual ~CSocketSession();

	virtual void		OnConnect(void);

	virtual void		OnClose(void);

	virtual void		OnError(unsigned int dwErrorNo);

	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);

	virtual void		Release(void);

	virtual char*		GetRecvBuf(){ return m_dataRecvBuf; }

	virtual unsigned int		GetRecvSize(){ return 64 * 1024; };

	//void				SetFxSessionFactory(IFxSessionFactory* pFxSessionFactory) { m_pFxSessionFactory = pFxSessionFactory; }

private:
	char m_dataRecvBuf[1024* 1024];
	//IFxSessionFactory* m_pFxSessionFactory;
};


class CSessionFactory : public TSingleton<CSessionFactory>, public IFxSessionFactory
{
public:
	CSessionFactory();
	virtual ~CSessionFactory(){}

	virtual FxSession*	CreateSession();

	void Init(){}
	virtual void Release(FxSession* pSession);

	std::set<FxSession*> m_setSessions;

private:
//	TDynamicPoolEx<CSocketSession> m_poolSessions;

	std::deque<FxSession* > m_listSession;

	FxCriticalLock		m_oLock;
};

class CWebSocketSessionFactory : public TSingleton<CWebSocketSessionFactory>, public IFxSessionFactory
{
public:
	CWebSocketSessionFactory();
	~CWebSocketSessionFactory(){}

	virtual FxSession*	CreateSession();

	void Init() {}
	virtual void Release(FxSession* pSession);

	std::set<FxSession*> m_setSessions;

private:
	//	TDynamicPoolEx<CSocketSession> m_poolSessions;

	std::deque<FxSession*> m_listSession;

	FxCriticalLock		m_oLock;
};

//static CSessionFactory oSessionFactory;

class BinaryDataHeader : public IFxDataHeader
{
public:
	BinaryDataHeader();
	virtual ~BinaryDataHeader();
	virtual unsigned int GetHeaderLength(){ return sizeof(m_dataRecvBuffer); }		// 消息头长度
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(unsigned int& dwHeaderLen, unsigned int dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, unsigned int dwLen, unsigned int dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
private:
	// // 消息头 为网络字节序
	char m_dataRecvBuffer[8];
	char m_dataSendBuffer[8];
	static const unsigned int s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
	//static const UINT32 s_dwMagic = 12345678;
};

class WebSocketDataHeader : public IFxDataHeader
{
public:
	WebSocketDataHeader();
	virtual ~WebSocketDataHeader();
	virtual unsigned int GetHeaderLength() { return m_dwHeaderLength; }		// 消息头长度 这个只能BuildRecvPkgHeader之后调用
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(unsigned int& dwHeaderLen, unsigned int dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, unsigned int dwLen, unsigned int dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
	virtual int	ParsePacket(const char* pBuf, unsigned int dwLen);

private:
	/************************/
	/* 0x0		附加数据帧	*/
	/* 0x1		文本		*/
	/* 0x2		二进制		*/
	/* 0x3-7	保留		*/
	/* 0x8		关闭		*/
	/* 0x9		ping		*/
	/* 0xA		pong		*/
	/* 0xB-F	保留		*/
	/************************/
	unsigned char m_btOpCode;
	unsigned char m_btFin;
	unsigned char m_btMask;
	unsigned long long m_qwPayloadLen;
	unsigned char m_btMaskingKey[4];
private:
	// // 消息头 为网络字节序
	char m_dataRecvBuffer[16];
	char m_dataSendBuffer[16];
	static const unsigned int s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';

	unsigned int m_dwHeaderLength;
	//static const UINT32 s_dwMagic = 12345678;
};

class CBinarySocketSession : public CSocketSession
{
public:
	CBinarySocketSession()
	{
	}

	~CBinarySocketSession()
	{
	}

	virtual IFxDataHeader* GetDataHeader() { return &m_oBinaryDataHeader; }
	virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;
};

class CWebSocketSession : public CSocketSession
{
public:
	CWebSocketSession()
	{
	}

	~CWebSocketSession()
	{
	}

	virtual IFxDataHeader* GetDataHeader() { return &m_oWebSocketDataHeader; }
	virtual void Release(void);
private:
	WebSocketDataHeader m_oWebSocketDataHeader;
};

class CChatManagerSession : public TSingleton<CChatManagerSession>, public CBinarySocketSession, public CEventCaller<CChatManagerSession, 1>
{
public:
	virtual void		OnConnect(void);
	virtual void		OnRecv(const char* pBuf, unsigned int dwLen);
	virtual void		Release(void);
	virtual void		OnClose();

	virtual bool OnTimer(double fSecond);
protected:
private:
};

//static CChatManagerSession g_oChatManagerSession;


#endif // !__SocketSession_H__
