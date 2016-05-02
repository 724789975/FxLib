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

private:

};

static CSessionFactory oSessionFactory;

class DataHeader : public IFxDataHeader
{
public:
	DataHeader();
	virtual ~DataHeader();
	virtual unsigned int GetHeaderLength(){ return sizeof(m_dataBuffer); }		// ?????
	virtual void* GetPkgHeader();
	virtual void* BuildSendPkgHeader(UINT32 dwDataLen);
	virtual bool BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset);
	virtual int __CheckPkgHeader(const char* pBuf);
private:
	// 消息头 为网络字节序
	char m_dataBuffer[8];
	static const UINT32 s_dwMagic = 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T';
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
