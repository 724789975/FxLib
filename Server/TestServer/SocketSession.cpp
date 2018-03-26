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

void CSocketSession::OnError(UINT32 dwErrorNo)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, connect addr : %p, error no : %d", GetRemoteIPStr(), GetRemotePort(), (GetConnection()), dwErrorNo);
}

class DBQuery : public IQuery
{
public:
	DBQuery(){}
	virtual ~DBQuery(){}

	virtual INT32 GetDBId(void) { return 0; }

	virtual void OnQuery(IDBConnection *poDBConnection)
	{
		IDataReader* pReader = NULL;
		if (poDBConnection->Query(m_strQuery.c_str(), &pReader) == FXDB_HAS_RESULT)
		{
			while(pReader->GetNextRecord())
			{
				char strValue[1024] = { 0 };
				UINT32 dwLen = 0;
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
void CSocketSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s", GetRemoteIPStr(), GetRemotePort(), pBuf);

	HttpRequestInfo oInfo = {0};
	HttpHelp::parse_http_request(const_cast<char*>(pBuf), dwLen, &oInfo);
	//printf("time : %s ip : %s, port : %d, recv %s", GetTimeHandler()->GetTimeStr(), GetRemoteIPStr(), GetRemotePort(), pBuf);

	//CNetStream oStream(pBuf, dwLen);
	//std::string szPlayerId;
	//oStream.ReadString(szPlayerId);

	//char szPlayerInfo[1024];
	//CNetStream oGameStream(ENetStreamType_Write, szPlayerInfo, 1024);
	//oGameStream.WriteInt(40002);
	//oGameStream.WriteString(szPlayerId);
	//mapSocket[szPlayerId] = this;
	//CChatManagerSession::Instance()->Send(szPlayerInfo, 1024 - oGameStream.GetDataLength());

	std::string szBuf = "<!DOCTYPE html PUBLIC \" -//W3C//DTD XHTML 1.0 Stict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n"
		"<html xmlns = \"http://www.w3.org/1999/xhtml\" lang = \"zh-CN\">\r\n"
		"<html>\r\n"
		"<head>\r\n"
		"<meta http-equiv=\"Content - Type\" content=\"text / html; charset = \"UTF-8\">\r\n"
		"<title>Test</title>\r\n"
		"<body>\r\n"
		"</body>\r\n"
		"</html>\r\n";

	if (!Send(szBuf.c_str(), szBuf.size()))
	{
		LogExe(LogLv_Debug, "ip : %s, port : %d, recv %s send error", GetRemoteIPStr(), GetRemotePort(), pBuf);
		Close();
	}

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
	m_pLock = FxCreateThreadLock();
	for(int i = 0; i < 512; ++i)
	{
		//CBinarySocketSession* pSession = new CBinarySocketSession;
		CSocketSession* pSession = new CSocketSession;
		m_listSession.push_back(pSession);
	}
//	m_poolSessions.Init(100, 10, false, 100);
}

FxSession*	CSessionFactory::CreateSession()
{
	m_pLock->Lock();
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
	m_pLock->UnLock();
	return pSession;
}

void CSessionFactory::Release(FxSession* pSession)
{
	m_pLock->Lock();
//	m_poolSessions.ReleaseObj(pSession);
	m_listSession.push_back(pSession);
	m_setSessions.erase(pSession);
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_pLock->UnLock();
}

CWebSocketSessionFactory::CWebSocketSessionFactory()
{
	m_pLock = FxCreateThreadLock();
	for (int i = 0; i < 64; ++i)
	{
		CWebSocketSession* pSession = new CWebSocketSession;
		m_listSession.push_back(pSession);
	}
}

FxSession * CWebSocketSessionFactory::CreateSession()
{
	m_pLock->Lock();
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
	m_pLock->UnLock();
	return pSession;
}

void CWebSocketSessionFactory::Release(FxSession * pSession)
{
	m_pLock->Lock();
	//	m_poolSessions.ReleaseObj(pSession);
	m_listSession.push_back(pSession);
	m_setSessions.erase(pSession);
	LogExe(LogLv_Debug, "left free session : %d", (int)m_listSession.size());
	m_pLock->UnLock();
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

void* BinaryDataHeader::BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen)
{
	//*((UINT32*)m_dataBuffer) = htonl(dwDataLen);
	dwHeaderLen = sizeof(m_dataSendBuffer);
	CNetStream oNetStream(ENetStreamType_Write, m_dataSendBuffer, sizeof(m_dataSendBuffer));
	oNetStream.WriteInt(dwDataLen);
	oNetStream.WriteInt(s_dwMagic);
	return (void*)m_dataSendBuffer;
}

bool BinaryDataHeader::BuildRecvPkgHeader(char* pBuff, UINT32 dwLen, UINT32 dwOffset)
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

	UINT32 dwHeaderLength = 0;
	UINT32 dwBufferLength = 0;
	oHeaderStream.ReadInt(dwHeaderLength);
	oRecvStream.ReadInt(dwBufferLength);

	UINT32 dwHeaderMagic = 0;
	UINT32 dwBufferMagic = 0;
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

void* WebSocketDataHeader::BuildSendPkgHeader(UINT32& dwHeaderLen, UINT32 dwDataLen)
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

bool WebSocketDataHeader::BuildRecvPkgHeader(char * pBuff, UINT32 dwLen, UINT32 dwOffset)
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

int WebSocketDataHeader::ParsePacket(const char * pBuf, UINT32 dwLen)
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

void CChatManagerSession::OnConnect(void)
{
	char szPlayerInfo[1024];
	CNetStream oGameStream(ENetStreamType_Write, szPlayerInfo, 1024);
	oGameStream.WriteInt(40001);
	oGameStream.WriteString("test");
	Send(szPlayerInfo, 1024 - oGameStream.GetDataLength());
}

void CChatManagerSession::OnRecv(const char* pBuf, UINT32 dwLen)
{
	CNetStream oStream(pBuf, dwLen);
	UINT32 dwProtocol = 0;
	oStream.ReadInt(dwProtocol);
	if (dwProtocol != 45002)
	{
		return;
	}
	std::string szPlayerId;
	oStream.ReadString(szPlayerId);
	std::string szIp;
	oStream.ReadString(szIp);
	std::string szSign;
	oStream.ReadString(szSign);
	UINT32 dwPort = 0;
	oStream.ReadInt(dwPort);
	UINT32 dwWebPort = 0;
	oStream.ReadInt(dwWebPort);

	if (mapSocket.find(szPlayerId) == mapSocket.end())
	{
		LogExe(LogLv_Critical, "error");
		return;
	}

	mapSocket[szPlayerId]->Send(pBuf, dwLen);
}

void CChatManagerSession::Release(void)
{
	CSocketSession::Release();
}

void CChatManagerSession::OnClose()
{
	GetTimeHandler()->AddDelayTimer(2, this);
}

bool CChatManagerSession::OnTimer(double fSecond)
{
	FxNetGetModule()->TcpConnect(this, inet_addr("127.0.0.1"), 13001, true);
	return true;
}
