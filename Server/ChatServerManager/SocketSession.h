#ifndef __SocketSession_H__
#define __SocketSession_H__
#include "dynamicpoolex.h"

#include "ifnet.h"

#include <set>
#include <list>
#include <deque>

class TextDataHeader : public IFxDataHeader
{
public:
	TextDataHeader();
	virtual ~TextDataHeader();
	virtual unsigned int GetHeaderLength() { return 0; }		// 消息头长度
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
	virtual int	ParsePacket(const char* pBuf, UINT32 dwLen);
private:
	// // 消息头 为网络字节序
	//static const UINT32 s_dwMagic = 12345678;
};

class BinaryDataHeader : public IFxDataHeader
{
public:
	BinaryDataHeader();
	virtual ~BinaryDataHeader();
	virtual unsigned int GetHeaderLength(){ return sizeof(m_dataRecvBuffer); }		// 消息头长度
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
private:
	// // 消息头 为网络字节序
	char m_dataRecvBuffer[8];
	char m_dataSendBuffer[8];
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
	virtual void* BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen);
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
	unsigned char m_btOpCode;
	unsigned char m_btFin;
	unsigned char m_btMask;
	unsigned long long m_qwPayloadLen;
	unsigned char m_btMaskingKey[4];
private:
	// // 消息头 为网络字节序
	char m_dataRecvBuffer[16];
	char m_dataSendBuffer[16];
	static const UINT32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';

	unsigned int m_dwHeaderLength;
	//static const UINT32 s_dwMagic = 12345678;
};

class CHttpSession : public FxSession
{
public:
	CHttpSession();
	virtual ~CHttpSession();

	virtual void		OnConnect(void);

	virtual void		OnClose(void);

	virtual void		OnError(UINT32 dwErrorNo);

	virtual void		OnRecv(const char* pBuf, UINT32 dwLen);

	virtual void		Release(void);

	virtual char*		GetRecvBuf() { return m_dataRecvBuf; }

	virtual UINT32		GetRecvSize() { return 64 * 1024; };

	virtual IFxDataHeader* GetDataHeader() { Assert(0); return NULL; }

private:
	char m_dataRecvBuf[1024 * 1024];
};


class CHttpSessionFactory : public TSingleton<CHttpSessionFactory>, public IFxSessionFactory
{
public:
	CHttpSessionFactory();
	virtual ~CHttpSessionFactory() {}

	virtual FxSession*	CreateSession();

	virtual void Release(FxSession* pSession) { Assert(0); }
	virtual void Release(CHttpSession* pSession);

private:
	TDynamicPoolEx<CHttpSession> m_poolSessions;
};

#endif // !__SocketSession_H__
