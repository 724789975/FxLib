#ifndef __SocketSession_H__
#define __SocketSession_H__

#include "ifnet.h"

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
	char m_dataRecvBuf[64 * 1024];
};

class CSessionFactory : public IFxSessionFactory
{
public:
	CSessionFactory(){}
	virtual ~CSessionFactory(){}

	virtual FxSession*	CreateSession();
	virtual void		Release(FxSession* pSession){}

private:

};

static CSessionFactory oSessionFactory;

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
	// 消息头 为网络字节序
	char m_dataRecvBuffer[8];
	char m_dataSendBuffer[8];
	static const UINT32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
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
	//virtual void Release(void);
private:
	BinaryDataHeader m_oBinaryDataHeader;
};

#endif // !__SocketSession_H__
