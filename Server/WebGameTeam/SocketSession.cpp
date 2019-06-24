#include "SocketSession.h"
#include <string>
#include <stdio.h>
#include "fxdb.h"
#include "netstream.h"
#include <map>

CSocketSession::CSocketSession()
{
}


CSocketSession::~CSocketSession()
{
}

void CSocketSession::OnConnect(void)
{
//	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

void CSocketSession::OnClose(void)
{
//	LogFun(LT_Screen, LogLv_Debug, "ip : %s, port : %d", GetRemoteIPStr(), GetRemotePort());
}

void CSocketSession::OnError(unsigned int dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

class DBQuery : public IQuery
{
public:
	DBQuery(){}
	virtual ~DBQuery(){}

	virtual int GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		IDataReader* pReader = NULL;
		if (poDBConnection->Query(m_strQuery.c_str(), &pReader) == FXDB_HAS_RESULT)
		{
			while(pReader->GetNextRecord())
			{
				char strValue[1024] = { 0 };
				unsigned int dwLen = 0;
				dwLen += sprintf(strValue + dwLen, "%s", "role id : ");
				dwLen += sprintf(strValue + dwLen, "%s", pReader->GetFieldValue(0));
				dwLen += sprintf(strValue + dwLen, "%s", " user id : ");
				dwLen += sprintf(strValue + dwLen, "%s", pReader->GetFieldValue(1));
				dwLen += sprintf(strValue + dwLen, "%s", " role name ");
				dwLen += sprintf(strValue + dwLen, "%s", pReader->GetFieldValue(2));
				dwLen += sprintf(strValue + dwLen, "%s", " clan ");
				dwLen += sprintf(strValue + dwLen, "%s", pReader->GetFieldValue(3));

				pSession->Send(strValue, 1024);
			}
			pReader->Release();
		}
	}

	virtual void OnResult(void)
	{}

	virtual void Release(void)
	{
		delete this;
	}

	std::string m_strQuery;
	CSocketSession* pSession;

private:

};

std::map<std::string, CSocketSession*> mapSocket;
void CSocketSession::OnRecv(const char* pBuf, unsigned int dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), pBuf);
	//printf("time : %s ip : %s, port : %d, recv %s", GetTimeHandler()->GetTimeStr(), GetRemoteIPStr(), GetRemotePort(), pBuf);

	CNetStream oStream(pBuf, dwLen);
	std::string szPlayerId;
	oStream.ReadString(szPlayerId);

	char szPlayerInfo[1024];
	CNetStream oGameStream(ENetStreamType_Write, szPlayerInfo, 1024);
	oGameStream.WriteInt(40002);
	oGameStream.WriteString(szPlayerId);
	mapSocket[szPlayerId] = this;
	//CChatManagerSession::Instance()->Send(szPlayerInfo, 1024 - oGameStream.GetDataLength());

	//if (!Send(pBuf, dwLen))
	//{
	//	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s send error", GetRemoteIPStr(), GetRemotePort(), pBuf);
	//	Close();
	//}

	//DBQuery * pQuery = new DBQuery;
	//pQuery->m_strQuery = pBuf;
	//FxDBGetModule()->AddQuery(pQuery);
	//pQuery->pSession = this;
}

void CSocketSession::Release(void)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p", GetRemoteIPStr(), GetRemotePort(), GetConnection());
	OnDestroy();

	Init(NULL);
}

CSessionFactory::CSessionFactory()
{
	for(int i = 0; i < 64; ++i)
	{
		CBinarySocketSession* pSession = new CBinarySocketSession;
		m_listSession.push_back(pSession);
	}
//	m_poolSessions.Init(100, 10, false, 100);
}

FxSession*	CSessionFactory::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	if(m_listSession.size() > 0)
	{
		pSession = *(m_listSession.begin());
		m_listSession.pop_front();
	}
	if(pSession)
	{
		m_setSessions.insert(pSession);
	}
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_oLock.UnLock();
	return pSession;
}

void CSessionFactory::Release(FxSession* pSession)
{
	m_oLock.Lock();
//	m_poolSessions.ReleaseObj(pSession);
	m_listSession.push_back(pSession);
	m_setSessions.erase(pSession);
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_oLock.UnLock();
}

CWebSocketSessionFactory::CWebSocketSessionFactory()
{
	for (int i = 0; i < 64; ++i)
	{
		CWebSocketSession* pSession = new CWebSocketSession;
		m_listSession.push_back(pSession);
	}
}

FxSession * CWebSocketSessionFactory::CreateSession()
{
	m_oLock.Lock();
	FxSession* pSession = NULL;
	if (m_listSession.size() > 0)
	{
		pSession = *(m_listSession.begin());
		m_listSession.pop_front();
	}
	if (pSession)
	{
		m_setSessions.insert(pSession);
	}
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_oLock.UnLock();
	return pSession;
}

void CWebSocketSessionFactory::Release(FxSession * pSession)
{
	m_oLock.Lock();
	//	m_poolSessions.ReleaseObj(pSession);
	m_listSession.push_back(pSession);
	m_setSessions.erase(pSession);
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_oLock.UnLock();
}

BinaryDataHeader::BinaryDataHeader()
{
}

BinaryDataHeader::~BinaryDataHeader()
{

}

void* BinaryDataHeader::GetPkgHeader()
{
	return (void*)m_dataRecvBuffer;
}

void* BinaryDataHeader::BuildSendPkgHeader(unsigned int& dwHeaderLen, unsigned int dwDataLen)
{
	//*((UINT32*)m_dataBuffer) = htonl(dwDataLen);
	dwHeaderLen = sizeof(m_dataSendBuffer);
	CNetStream oNetStream(ENetStreamType_Write, m_dataSendBuffer, sizeof(m_dataSendBuffer));
	oNetStream.WriteInt(dwDataLen);
	oNetStream.WriteInt(s_dwMagic);
	return (void*)m_dataSendBuffer;
}

bool BinaryDataHeader::BuildRecvPkgHeader(char* pBuff, unsigned int dwLen, unsigned int dwOffset)
{
	if (dwLen + dwOffset > GetHeaderLength())
	{
		return false;
	}

	memcpy(m_dataRecvBuffer + dwOffset, pBuff, dwLen);
	return true;
}

int BinaryDataHeader::__CheckPkgHeader(const char* pBuf)
{
	CNetStream oHeaderStream(m_dataRecvBuffer, sizeof(m_dataRecvBuffer));
	CNetStream oRecvStream(pBuf, sizeof(m_dataRecvBuffer));

	unsigned int dwHeaderLength = 0;
	unsigned int dwBufferLength = 0;
	oHeaderStream.ReadInt(dwHeaderLength);
	oRecvStream.ReadInt(dwBufferLength);

	unsigned int dwHeaderMagic = 0;
	unsigned int dwBufferMagic = 0;
	oHeaderStream.ReadInt(dwHeaderMagic);
	oRecvStream.ReadInt(dwBufferMagic);

	if (dwHeaderLength != dwBufferLength)
	{
		return -1;
	}

	if (s_dwMagic != dwBufferMagic)
	{
		return -1;
	}

	return (GetHeaderLength() + dwHeaderLength);
}

WebSocketDataHeader::WebSocketDataHeader()
	:m_dwHeaderLength(0)
{
}

WebSocketDataHeader::~WebSocketDataHeader()
{
}

void * WebSocketDataHeader::GetPkgHeader()
{
	return (void*)m_dataRecvBuffer;
}

void* WebSocketDataHeader::BuildSendPkgHeader(unsigned int& dwHeaderLen, unsigned int dwDataLen)
{
	dwHeaderLen = 1;
	CNetStream oNetStream(ENetStreamType_Write, m_dataSendBuffer, sizeof(m_dataSendBuffer));
	unsigned char btFinOpCode = 0x82;
	// unsigned char btFin = (btFinOpCode >> 7) & 0xff;
	// unsigned char btOpCode = (btFinOpCode) & 0x0f;
	oNetStream.WriteByte(btFinOpCode);
	if (dwDataLen < 126 && dwDataLen <= 0xFFFF)
	{
		unsigned char btLen = dwDataLen;
		oNetStream.WriteByte(btLen);
		dwHeaderLen += 1;
	}
	else if (dwDataLen >= 126 && dwDataLen <= 0xFFFF)
	{
		unsigned char btLen = 126;
		oNetStream.WriteByte(btLen);
		unsigned short wLen = dwDataLen;
		oNetStream.WriteShort(wLen);
		dwHeaderLen += 3;
	}
	else
	{
		unsigned char btLen = 127;
		oNetStream.WriteByte(btLen);
		unsigned long long qwLen = dwDataLen;
		oNetStream.WriteInt64(qwLen);
		dwHeaderLen += 9;
	}
	return (void*)m_dataSendBuffer;
}

bool WebSocketDataHeader::BuildRecvPkgHeader(char * pBuff, unsigned int dwLen, unsigned int dwOffset)
{
	memcpy(m_dataRecvBuffer + dwOffset, pBuff, sizeof(m_dataRecvBuffer) - dwOffset > dwLen ? dwLen : sizeof(m_dataRecvBuffer) - dwOffset);
	m_dwHeaderLength = 0;
	if (dwLen + dwOffset > 2)
	{
		__CheckPkgHeader(m_dataRecvBuffer);
	}
	return true;
}

int WebSocketDataHeader::__CheckPkgHeader(const char * pBuf)
{
	m_dwHeaderLength = 0;
	CNetStream oHeaderStream(pBuf, sizeof(m_dataRecvBuffer));
	
	unsigned char bt1 = 0, bt2 = 0;
	oHeaderStream.ReadByte(bt1);
	oHeaderStream.ReadByte(bt2);
	m_dwHeaderLength += 2;
	m_btFin = (bt1 >> 7) & 0xff;
	m_btOpCode = (bt1) & 0x0f;
	m_btMask = (bt2 >> 7) & 0xff;
	m_qwPayloadLen = bt2 & 0x7f;

	if (m_qwPayloadLen == 126)
	{
		unsigned short wTemp = 0;
		oHeaderStream.ReadShort(wTemp);
		m_qwPayloadLen = wTemp;
		m_dwHeaderLength += 2;
	}
	else if (m_qwPayloadLen == 127)
	{
		unsigned long long qwTemp = 0;
		oHeaderStream.ReadInt64(qwTemp);
		m_qwPayloadLen = qwTemp;
		m_dwHeaderLength += 8;
	}
	
	if (m_btMask)
	{
		memcpy(m_btMaskingKey, oHeaderStream.ReadData(sizeof(m_btMaskingKey)), sizeof(m_btMaskingKey));
		m_dwHeaderLength += 4;
	}

	return (int)(m_qwPayloadLen + m_dwHeaderLength);
}

int WebSocketDataHeader::ParsePacket(const char * pBuf, unsigned int dwLen)
{
	if (dwLen < 2)
	{
		return 0;
	}

	int nPkgLen = __CheckPkgHeader(m_dataRecvBuffer);

	if (dwLen < m_dwHeaderLength)
	{
		return 0;
	}

	return nPkgLen;
}

void CBinarySocketSession::Release(void)
{
	CSocketSession::Release();
	CSessionFactory::Instance()->Release(this);
}

void CWebSocketSession::Release(void)
{
	CSocketSession::Release();
	CWebSocketSessionFactory::Instance()->Release(this);
}

