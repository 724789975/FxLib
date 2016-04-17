#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "ifnet.h"

class FxConnectSock;

class FxConnection
{
	enum EConnStat
	{
		CONN_NONE = 0,
		CONN_OK,
		CONN_ASSOCIATE,
		CONN_CLOSING,
	};

public:
	FxConnection();
	virtual ~FxConnection();

	bool						IsConnected(void);
	bool						Send(const char* pBuf,UINT32 dwLen);
	void						Close(void);
	bool						Reconnect();
	const UINT32				GetRemoteIP(void)		 { return m_dwRemoteIP;	}
	const char*					GetRemoteIPStr(void)	{ return m_szRemoteIP;	}
	UINT16						GetRemotePort(void)		{ return m_wRemotePort; }
	const UINT32				GetLocalIP(void)		{ return m_dwLocalIP;	}
	const char*					GetLocalIPStr(void)		 { return m_szLocalIP;	}
	UINT16						GetLocalPort(void)		{ return m_wLocalPort;	}

	void						SetRemoteIP(UINT32 dwIP);
	void						SetRemotePort(UINT16 wPort)	{ m_wRemotePort = wPort; }
	void						SetLocalIP(UINT32 dwIP);
	void						SetLocalPort(UINT16 wPort)	{ m_wLocalPort = wPort; }
	void						SetSock(FxConnectSock* poSock);
	void						SetSession(FxSession* poSession)	{ m_poSession = poSession; }
	void						SetReconnect(bool bReconnect);

	bool						SetConnectionOpt(ESessionOpt eOpt, bool bSetting);

	void						Reset();

	void						SetID(UINT32 dwID)		{ m_dwID = dwID; }
	UINT32						GetID()						{ return m_dwID; }

	void						OnConnect();
	void						OnAssociate();
	void						OnClose();
	void						OnRecv(UINT32 dwLen);
	void						OnConnError(UINT32 dwErrorNo);
	void						OnError(UINT32 dwErrorNo);
	char*						GetRecvBuf();
	UINT32						GetRecvSize();

	void						OnSocketDestroy();

	IFxDataHeader*				GetDataHeader();

private:
	bool						m_bReconnect;
	UINT32						m_dwID;
	EConnStat					m_nConnStat;
	UINT32						m_dwLocalIP;
	UINT16						m_wLocalPort;
	UINT32						m_dwRemoteIP;
	UINT16						m_wRemotePort;
	char						m_szLocalIP[16];
	char						m_szRemoteIP[16];
	FxConnectSock*			m_poSock;
	FxSession*					m_poSession;
};

#endif	// __CONNECTION_H__
