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

	virtual void		OnError(UINT32 dwErrorNo);

	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);

	virtual void		Release(void);

	virtual char*		GetRecvBuf(){ return m_dataRecvBuf; }

	virtual UINT32		GetRecvSize(){ return 64 * 1024; };

private:
	char m_dataRecvBuf[1024* 1024];
};

class CSessionFactory : public IFxSessionFactory
{
public:
	CSessionFactory();
	virtual ~CSessionFactory(){}

	DECLARE_SINGLETON(CSessionFactory)

	virtual FxSession*	CreateSession();

	void Init(){}
	void Release(CSocketSession* pSession);

	std::set<FxSession*> m_setSessions;

private:
//	TDynamicPoolEx<CSocketSession> m_poolSessions;

	std::deque<CSocketSession* > m_listSession;

	IFxLock*			m_pLock;
};

//static CSessionFactory oSessionFactory;

class DataHeader : public IFxDataHeader
{
public:
	DataHeader();
	virtual ~DataHeader();
	virtual unsigned int GetHeaderLength(){ return sizeof(m_dataBuffer); }		// 消息头长度
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(UINT32 dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
private:
	// // 消息头 为网络字节序
	char m_dataBuffer[8];
	static const UINT32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
	//static const UINT32 s_dwMagic = 12345678;
};

class WebSocketDataHeader : public IFxDataHeader
{
public:
	WebSocketDataHeader();
	virtual ~WebSocketDataHeader();
	virtual unsigned int GetHeaderLength() { return m_dwHeaderLength; }		// 消息头长度 这个只能BuildRecvPkgHeader之后调用
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(UINT32 dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
	virtual int	ParsePacket(const char* pBuf, UINT32 dwLen);

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
	unsigned char m_ucOpCode;
	unsigned char m_ucFin;
	unsigned char m_ucMask;
	unsigned long long m_ullPayloadLen;
	unsigned char m_ucMaskingKey[4];
private:
	// // 消息头 为网络字节序
	char m_dataBuffer[16];
	static const UINT32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';

	unsigned int m_dwHeaderLength;
	//static const UINT32 s_dwMagic = 12345678;
};

class DataHeaderFactory : public IFxDataHeaderFactory
{
public:
	DataHeaderFactory(){}
	virtual ~DataHeaderFactory(){}
	virtual IFxDataHeader* CreateDataHeader(){ return new DataHeader; }
private:

};

static DataHeaderFactory oDataHeaderFactory;

#endif // !__SocketSession_H__
