#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "ifnet.h"

class FxTCPConnectSock;

class FxConnection
{
	enum EConnStat
	{
		CONN_NONE = 0,
		CONN_CONN,
		CONN_OK,
		CONN_ASSOCIATE,
		CONN_CLOSING,
	};

public:
	FxConnection();
	virtual ~FxConnection();

	bool						IsConnected(void);
	bool						IsConnecting(void);
	bool						Send(const char* pBuf,unsigned int dwLen);
	void						Close(void);
	SOCKET						Reconnect();
	const unsigned int			GetRemoteIP(void)				{ return m_dwRemoteIP;	}
	const char*					GetRemoteIPStr(void)			{ return m_szRemoteIP;	}
	unsigned short				GetRemotePort(void)				{ return m_wRemotePort; }
	const unsigned int			GetLocalIP(void)				{ return m_dwLocalIP;	}
	const char*					GetLocalIPStr(void)				{ return m_szLocalIP;	}
	unsigned short				GetLocalPort(void)				{ return m_wLocalPort;	}

	void						SetRemoteIP(unsigned int dwIP);
	void						SetRemotePort(unsigned short wPort)		{ m_wRemotePort = wPort; }
	void						SetLocalIP(unsigned int dwIP);
	void						SetLocalPort(unsigned short wPort)		{ m_wLocalPort = wPort; }
	void						SetSock(IFxConnectSocket* poSock);
	void						SetSession(FxSession* poSession){ m_poSession = poSession; }
	void						SetReconnect(bool bReconnect);
	void						SetSockType(ESocketType eType)	{ m_eSockType = eType; }
	bool						SetConnectionOpt(ESessionOpt eOpt, bool bSetting);

	void						Reset();

	void						SetID(unsigned int dwID)				{ m_dwID = dwID; }
	unsigned int				GetID()							{ return m_dwID; }

	void						OnConnect();
	void						OnAssociate();
	void						OnClose();
	void						OnRecv(unsigned int dwLen);
	void						OnConnError(unsigned int dwErrorNo);
	void						OnError(unsigned int dwErrorNo);
	char*						GetRecvBuf();
	unsigned int				GetRecvSize();

	void						OnSocketDestroy();

	IFxDataHeader*				GetDataHeader();

private:
	bool						m_bReconnect;
	unsigned int				m_dwID;
	EConnStat					m_nConnStat;
	unsigned int				m_dwLocalIP;
	unsigned short				m_wLocalPort;
	unsigned int				m_dwRemoteIP;
	unsigned short				m_wRemotePort;
	char						m_szLocalIP[16];
	char						m_szRemoteIP[16];
	IFxConnectSocket*			m_poSock;
	FxSession*					m_poSession;
	ESocketType					m_eSockType;
};

#endif	// __CONNECTION_H__
