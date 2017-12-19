/*
* FileName:	FxDB.h
* Author:		dengxiaobo	Version: FxLib V1.00	Date: 2015-4-10
* Description: DB模块头文件
* Version:	 V0.01
* History:
* 	<author>	<time>		<version>	<desc>
* 	<dengxiaobo>		<2015-4-10>	<V0.01>		创建
*/

#ifndef __FXREDIS_H_2017_1210__
#define __FXREDIS_H_2017_1210__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
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
#include <string>
#include <vector>

#include "fxmeta.h"

class	IRedisDataReader
{
public:
	virtual						~IRedisDataReader() {}

	virtual bool				GetValue(std::string& refszValue) = 0;

	virtual bool				GetValue(std::vector<std::string>& refszvecValue) = 0;

	virtual bool				GetValue(long long& refqwValue) = 0;

	virtual void				Release(void) = 0;
};


class IRedisConnection
{
public:
	virtual						~IRedisConnection() {}

	virtual int					Query(const char* pszCMD) = 0;

	virtual int					Query(const char* pszCMD, IRedisDataReader** ppReader) = 0;
};

class IRedisQuery
{
public:
	virtual						~IRedisQuery() {}

	virtual int					GetDBId(void) = 0;

	virtual void				OnQuery(IRedisConnection *poDBConnection) = 0;

	virtual void				OnResult(void) = 0;

	virtual void				Release(void) = 0;
};

class IRedisModule
{
public:
	virtual						~IRedisModule() {}

	virtual bool				Open(const char* szHost, unsigned int dwPort, unsigned int dwRedisId) = 0;

	virtual void				Close(unsigned int dwDBId) = 0;

	virtual bool				AddQuery(IRedisQuery *poQuery) = 0;

	virtual bool				Run(unsigned int dwCount = -1) = 0;
};

IRedisModule *					FxRedisGetModule();

#endif	// __FXREDIS_H_2017_1210__
