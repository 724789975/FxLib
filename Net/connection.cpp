#include "connection.h"
#include "mysock.h"
#include "sockmgr.h"
#include<string.h>
//#include "sockmgr.h"
//#include "connectionmgr.h"
//#include "connectthread.h"

#ifndef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif // WIN32


FxConnection::FxConnection()
{
	m_poSock = NULL;
	m_poSession = NULL;
	Reset();
}

FxConnection::~FxConnection()
{

}

void FxConnection::Reset()
{
	m_dwID			= 0;
	m_nConnStat		= CONN_NONE;
	m_dwLocalIP		= 0;
	m_wLocalPort	= 0;
	m_dwRemoteIP	= 0;
	m_wRemotePort	= 0;
	m_szLocalIP[0]	= '\0';
	m_szRemoteIP[0]	= '\0';
	if (m_poSock)
	{
		m_poSock->SetConnection(NULL);
		m_poSock = NULL;
	}
	if (m_poSession)
	{
		m_poSession->Init(NULL);
		m_poSession	= NULL;
	}
	m_bReconnect = false;
}

bool FxConnection::IsConnected(void)
{
	if (m_poSock)
	{
		return m_poSock->IsConnected() && (CONN_OK == m_nConnStat);
	}
	return false;
}

bool FxConnection::IsConnecting(void)
{
	return (CONN_ASSOCIATE == m_nConnStat || CONN_NONE == m_nConnStat);
}

void FxConnection::SetRemoteIP(UINT32 dwIP)
{
	m_dwRemoteIP = dwIP;
	memcpy(m_szRemoteIP, inet_ntoa((in_addr&)m_dwRemoteIP), 16);
}

void FxConnection::SetLocalIP(UINT32 dwIP)
{
	m_dwLocalIP = dwIP;
	memcpy(m_szLocalIP, inet_ntoa((in_addr&)m_dwLocalIP), 16);
}

bool FxConnection::Send(const char* pBuf,UINT32 dwLen)
{
	if (NULL == m_poSock)
	{
		return false;
	}

	if(m_nConnStat != CONN_OK)
	{
		return false;
	}

	if(NULL == pBuf || 0 == dwLen)
	{
		return false;
	}

	return m_poSock->Send(pBuf, dwLen);
}

void FxConnection::Close(void)
{
	if (NULL == m_poSock)
	{
		return;
	}

	if(m_nConnStat != CONN_OK)
	{
		return;
	}

	m_nConnStat = CONN_CLOSING;

#ifdef WIN32
	m_poSock->PostClose();
#else
	m_poSock->Close();
#endif //WIN32
}

void FxConnection::OnConnect()
{
	if (CONN_ASSOCIATE != m_nConnStat)
	{
		return;
	}

	m_nConnStat = CONN_OK;

	if (m_poSession == NULL)
	{
		return;
	}
	m_poSession->Init(this);
	m_poSession->OnConnect();
}

void FxConnection::OnAssociate()
{
	if (CONN_NONE != m_nConnStat)
	{
		return;
	}
	
	m_nConnStat = CONN_ASSOCIATE;
}

void FxConnection::OnClose()
{
	if (m_nConnStat == CONN_NONE)
	{
		if(m_poSession)
		{
			m_poSession->Release();
		}
		return;
	}

	if (m_poSession == NULL)
	{
		assert(0);
		return;
	}

	if(CONN_ASSOCIATE == m_nConnStat)
	{
		m_poSession->Init(this);
		m_poSession->OnConnect();
	}

	m_nConnStat = CONN_NONE;

	m_poSession->OnClose();
	m_poSession->Release();


//	FxMySockMgr::Instance()->DelayRelease(m_poSock);
//	if (m_poSock->IsAccept())
//	{
//		// �ǽ��ܷ�, �����𷽲���Ҫɾ��, ��Ҫ�õ�
//		m_poSession = NULL;
//		m_poSock->SetConnection(NULL);
//		FxConnectionMgr::Instance()->Release(this);
//	}
}

void FxConnection::OnRecv(UINT32 dwLen)
{
	if (NULL == m_poSession)
	{
		return;
	}

	if(m_nConnStat != CONN_OK)
		return;

	unsigned int dwHeaderLen = m_poSock->GetDataHeader()->GetHeaderLength();
	m_poSession->OnRecv(m_poSession->GetRecvBuf() + dwHeaderLen, dwLen - dwHeaderLen);
}

void FxConnection::OnError(UINT32 dwErrorNo)
{
	if (NULL == m_poSession)
	{
		return;
	}

	if(m_nConnStat != CONN_OK)
		return;

	m_poSession->OnError(dwErrorNo);
}

void FxConnection::SetReconnect(bool bReconnect)
{
	m_bReconnect = bReconnect;
}

char* FxConnection::GetRecvBuf()
{
	if (m_poSession)
	{
		return m_poSession->GetRecvBuf();
	}
	return NULL;
}

UINT32 FxConnection::GetRecvSize()
{
	if (m_poSession)
	{
		return m_poSession->GetRecvSize();
	}
	return 0;
}

void FxConnection::OnConnError(UINT32 dwErrorNo)
{
	if (NULL == m_poSession)
	{
		return;
	}

	m_poSession->OnError(dwErrorNo);

	if (m_bReconnect && m_poSock)
	{
		m_poSock->Connect();
	}
}

SOCKET FxConnection::Reconnect()
{
	if (m_poSock)
	{
		m_poSock->Close();
		return INVALID_SOCKET;
	}
	else
	{
		FxConnectSock* poSock = FxMySockMgr::Instance()->Create();
		if (NULL == poSock)
		{
			return INVALID_SOCKET;
		}
		poSock->SetConnection(this);
		SetSock(poSock);
		SetID(poSock->GetSockId());
		return poSock->Connect();
	}
}

bool FxConnection::SetConnectionOpt(ESessionOpt eOpt, bool bSetting)
{
	return false;
	//if (NULL == m_poSock)
	//{
	//	return false;
	//}

	//return m_poSock->SetSockOpt(eOpt, bSetting);
}

void FxConnection::SetSock(FxConnectSock* poSock)
{
	m_poSock = poSock;
}

void FxConnection::OnSocketDestroy()
{
	m_poSock = NULL;
}

IFxDataHeader* FxConnection::GetDataHeader()
{
	if (m_poSession)
	{
		return m_poSession->GetDataHeader();
	}
	return NULL;
}

