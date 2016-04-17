/*
 * FileName:    FxDB.h
 * Author:      dengxiaobo  Version: FxLib V1.00  Date: 2015-4-10
 * Description: DBģ��ͷ�ļ�   
 * Version:     V0.01
 * History:        
 * 	<author>	<time>		<version>	<desc>
 * 	<dengxiaobo>		<2015-4-10>	<V0.01>		����
 */

#ifndef __FXDB_H_2016_0303__
#define __FXDB_H_2016_0303__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <WinSock2.h>
#include <windows.h>
#else
#include <string.h>
#include <stdarg.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h> 
#include <unistd.h> 
#include <signal.h> 
#include <sys/time.h>
#include <errno.h>
#include <arpa/inet.h>
#endif

#include <assert.h>

#include "fxmeta.h"
#include <mysql.h>

#ifdef	FXN_DLLCLASS
#define	DLLCLASS_DECL			__declspec(dllimport)	
#else
#define	DLLCLASS_DECL			__declspec(dllexport)	
#endif

#ifdef FXN_STATICLIB
#undef	DLLCLASS_DECL
#define DLLCLASS_DECL
#endif

#define MAX_DBNAME_SIZE		            64
#define MAX_DBCHARACTSET_SIZE		    32

const char FXDB_MODULENAME[]        = "FXMySQLDB";

enum EFxDBCode
{
    FXDB_ERR_UNKNOWN    = -4,       // δ֪����
    FXDB_ERR_READER     = -3,       // ��������ݽ������
    FXDB_ERR_CONN       = -2,       // ���Ӵ���
    FXDB_DISCONNECT     = -1,       // �Ͽ�����
    FXDB_SUCCESS        = 0,        // �ɹ�
    FXDB_NO_RESULT      = 1,        // �ɹ����޽��
    FXDB_HAS_RESULT     = 2         // �ɹ����в�ѯ���
};

struct SDBAccount{
    char                m_szHostName[MAX_DBNAME_SIZE];        
    char                m_szDBName[MAX_DBNAME_SIZE];          
    char                m_szLoginName[MAX_DBNAME_SIZE];       
    char                m_szLoginPwd[MAX_DBNAME_SIZE];        
	char                m_szCharactSet[MAX_DBCHARACTSET_SIZE];  
    UINT16              m_wConnPort;
};

struct SDBConnInfo
{    
    UINT32              m_dwDBId;           
	SDBAccount          m_stAccount; 
};

class  IDataReader
{
public:
    virtual   ~IDataReader(){}
	
	virtual UINT32      GetRecordCount(void) = 0;
    
    virtual UINT32      GetFieldCount(void) = 0;
    
    virtual bool        GetFirstRecord(void) = 0;

    virtual bool        GetNextRecord(void) = 0;
	
    virtual const char* GetFieldValue(UINT32 dwIndex) = 0;
	
    virtual INT32       GetFieldLength(UINT32 dwIndex) = 0;
	
    virtual void        Release(void)=0;
};

class IStmt
{
public:
	virtual ~IStmt () {}

	virtual bool		Prepare(const char* pszSQL, UINT32 dwParamNum, UINT32 dwResultNum) = 0;

	virtual void		BindParam(enum_field_types eFieldType, void* pBuf, UINT32 dwSize) = 0;

	virtual bool		BindParamComplete() = 0;

	virtual void		BindResult(enum_field_types eFieldType, void* pBuf, UINT32 dwBufSize, UINT32* pdwLength) = 0;

	virtual bool		BindResultComplete() = 0;

	virtual bool		Execute() = 0;

	virtual const char* GetErrorString() = 0;

	virtual UINT32		AffectedRows() = 0;

	virtual bool		Restore() = 0;

	virtual bool		Fetch() = 0;

	virtual UINT32		NumOfRows() = 0;

	virtual void		Release() = 0;
};

class IDBConnection
{
public:	
    virtual   ~IDBConnection(){}

    virtual UINT32      EscapeString(const char* pszSrc, INT32 nSrcLen, char* pszDest, INT32 nDestLen) = 0;
    
	virtual INT32       Query(const char* pszSQL) = 0;
	
    virtual INT32       Query(const char* pszSQL, IDataReader** ppReader) = 0;
	
	virtual UINT32      GetLastError(void) = 0;
    
    virtual const char* GetLastErrorString(void) = 0;

	virtual IStmt*		CreateStmt(void) = 0;
};

class IQuery 
{
public:
    virtual   ~IQuery(){}

    virtual INT32       GetDBId(void) = 0;
	
	virtual void        OnQuery(IDBConnection *poDBConnection) = 0;
	
    virtual void        OnResult(void) = 0;
	
    virtual void        Release(void) = 0;
};

class IDBModule
{
public:
    virtual   ~IDBModule(){}

	virtual bool        Open(SDBConnInfo& DBInfo) = 0;
	
	virtual void        Close(UINT32 dwDBId) = 0;
    	
	virtual bool        AddQuery(IQuery *poQuery) = 0;
	
	virtual bool        Run(UINT32 dwCount = -1) = 0;
};

IDBModule *             FxDBGetModule();

//typedef IDBModule *     (*PFN_FxDBGetModule)();

//bool                    FxDBSetLogger(IFxLogger* poLogger, UINT32 dwLevel);

//typedef bool            (*PFN_FxDBSetLogger)(IFxLogger* poLogger, UINT32 dwLevel);

#endif  // __FXDB_H_2016_0303__
