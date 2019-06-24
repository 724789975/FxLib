#include "ifnet.h"
#include "connection.h"
#include "net.h"
#include "connectionmgr.h"

FxSession::FxSession()
{
	m_poConnection = NULL;
	//m_pDataHeader = NULL;
}

FxSession::~FxSession()
{
	if (m_poConnection)
	{
		m_poConnection->SetSession(NULL);
		FxConnectionMgr::Instance()->Release(m_poConnection);
		m_poConnection = NULL;
	}
	//if (m_pDataHeader)
	//{
	//	delete m_pDataHeader;
	//	m_pDataHeader = NULL;
	//}
}

bool FxSession::Send(const char* pBuf,unsigned int dwLen)
{
	if (!m_poConnection)
	{
		LogExe(LogLv_Error, "m_poConnection == NULL");
		return false;
	}
	if (m_poConnection->IsConnected())
	{
	    return m_poConnection->Send(pBuf, dwLen);
	}
	
	LogExe(LogLv_Error, "connection : %p, id : %d, IsConnected() : %d", m_poConnection, m_poConnection->GetID, (int)m_poConnection->IsConnected());
	return false;
}

void FxSession::Close(void)
{
	if (m_poConnection && m_poConnection->IsConnected())
	{
	    m_poConnection->Close();
	}
}

void FxSession::Init(FxConnection* poConnection)
{
	m_poConnection = poConnection;
	Init();
}

bool FxSession::SetSessionOpt(ESessionOpt eOpt, bool bSetting)
{
	if (NULL == m_poConnection)
	{
	    return false;
	}

	return m_poConnection->SetConnectionOpt(eOpt, bSetting);
}

bool FxSession::IsConnected(void)
{
	if (NULL == m_poConnection)
	{
	    return false;
	}

	return m_poConnection->IsConnected();
}

bool FxSession::IsConnecting(void)
{
	if (NULL == m_poConnection)
	{
		return false;
	}

	return m_poConnection->IsConnecting();
}

SOCKET FxSession::Reconnect(void)
{
	if (NULL == m_poConnection)
	{
		return INVALID_SOCKET;
	}

	return m_poConnection->Reconnect();
}

unsigned int FxSession::GetRemoteIP()
{
	if (NULL != m_poConnection)
	{
		return m_poConnection->GetRemoteIP();
	}

	return 0;
}

const char* FxSession::GetRemoteIPStr()
{
	if (NULL != m_poConnection)
	{
	    return m_poConnection->GetRemoteIPStr();
	}

	return "NoIP";
}

unsigned int FxSession::GetRemotePort()
{
	if (NULL != m_poConnection)
	{
	    return m_poConnection->GetRemotePort();
	}

	return 0;
}

FxConnection* FxSession::GetConnection(void)
{
	return m_poConnection;
}

bool FxSession::OnDestroy()
{
	//  connection release
	if (m_poConnection)
	{
		m_poConnection->SetSession(NULL);
		FxConnectionMgr::Instance()->Release(m_poConnection);
		m_poConnection = NULL;
	}

//	if (m_pDataHeader)
//	{
//		delete m_pDataHeader;
//		m_pDataHeader = NULL;
//	}
	return true;
}

//void FxSession::SetDataHeader(IFxDataHeader* pDataHeader)
//{
//	m_pDataHeader = pDataHeader;
//}

IFxNet* FxNetGetModule()
{
	if(NULL == FxNetModule::Instance())
	{
#ifdef WIN32
	    WSADATA data;
		int nErr = WSAStartup(MAKEWORD(2, 2), &data);
		if (nErr != 0)
		{
			LogExe(LogLv_Error, "WSAStartup failed error no : %d", nErr);
		}
#endif
	    if(false == FxNetModule::CreateInstance())
	        return NULL;

	     if(false == FxNetModule::Instance()->Init())
	     {
			 LogExe(LogLv_Error, "%s", "SDNetGetModule, Init CSDNetWin failed");
	         FxNetModule::DestroyInstance();
	         return NULL;
	     }
	}

	return FxNetModule::Instance();
}

int IFxDataHeader::ParsePacket(const char* pBuf, unsigned int dwLen)
{
	if (dwLen < GetHeaderLength())
	{
		return 0;
	}

	int iPkgLen = __CheckPkgHeader(pBuf);

	return iPkgLen;
}
