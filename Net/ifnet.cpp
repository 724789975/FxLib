#include "ifnet.h"
#include "connection.h"
#include "net.h"
#include "connectionmgr.h"

FxSession::FxSession()
{
    m_poConnection = NULL;
	m_pDataHeader = NULL;
}

FxSession::~FxSession()
{
	if (m_poConnection)
	{
		m_poConnection->SetSession(NULL);
		FxConnectionMgr::Instance()->Release(m_poConnection);
		m_poConnection = NULL;
	}
	if (m_pDataHeader)
	{
		delete m_pDataHeader;
		m_pDataHeader = NULL;
	}
}

bool FxSession::Send(const char* pBuf,UINT32 dwLen)
{
    if (m_poConnection && m_poConnection->IsConnected())
    {
        return m_poConnection->Send(pBuf, dwLen);
    }
    
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

bool FxSession::Reconnect(void)
{
	if (NULL == m_poConnection)
	{
		return false;
	}

	return m_poConnection->Reconnect();
}

const char* FxSession::GetRemoteIPStr()
{
    if (NULL != m_poConnection)
    {
        return m_poConnection->GetRemoteIPStr();
    }

    return "NoIP";
}

UINT32 FxSession::GetRemotePort()
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
	// 当销毁的时候 应该是吧connection 也release掉
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

void FxSession::SetDataHeader(IFxDataHeader* pDataHeader)
{
	m_pDataHeader = pDataHeader;
}

IFxNet* FxNetGetModule()
{
    if(NULL == FxNetModule::Instance())
    {
#ifdef WIN32
        WSADATA data;
		int nErr = WSAStartup(MAKEWORD(2, 2), &data);
		if (nErr != 0)
		{
			LogScreen("WSAStartup failed error no : %d", nErr);
		}
#endif
        if(false == FxNetModule::CreateInstance())
            return NULL;

         if(false == FxNetModule::Instance()->Init())
         {
			 LogScreen("SDNetGetModule, Init CSDNetWin failed");
             FxNetModule::DestroyInstance();
             return NULL;
         }
    }

    return FxNetModule::Instance();
}

