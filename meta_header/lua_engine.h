/*
 * lua_engine.h
 *
 *  Created on: 2015??11??9??
 *      Author: dengxiaobo
 */

#ifndef __LUA_ENGINE_H__
#define __LUA_ENGINE_H__

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
;

#include "tolua++.h"
#include "fxmeta.h"
#include <vector>
#include <assert.h>
#include <typeinfo>
#include <string>

typedef int ToluaFunctionOpen(lua_State*);

#define WORK_PATH "scripts"

#ifdef WIN32

#ifdef	FXN_DLLCLASS
#define	DLLCLASS_DECL			__declspec(dllimport)
#else
#define	DLLCLASS_DECL			__declspec(dllexport)
#endif

#ifdef FXN_STATICLIB
#undef	DLLCLASS_DECL
#define DLLCLASS_DECL
#endif

#else
#define DLLCLASS_DECL
#endif

class DLLCLASS_DECL CLuaEngine: public ListDirAndLoadFile
{
DECLARE_SINGLETON(CLuaEngine)
public:
	CLuaEngine();
	virtual ~CLuaEngine();

	bool Init(std::vector<ToluaFunctionOpen*>& vecToLuaFunctions);
	bool Reload();

	void Close();
	lua_State* GetLuaState();

	void CallVoidFunction(const char* pFunctionName);
	double CallNumberFunction(const char* pFunctionName);
	const char* CallStringFunction(const char* pFunctionName);

	template<typename TRet>
	TRet* CallUserFunction(const char* pFunctionName);

	template<typename T>
	void CallVoidFunction(const char* pFunctionName, T tParam);

	template<typename T>
	double CallNumberFunction(const char* pFunctionName, T tParam);

	template<typename T>
	const char* CallStringFunction(const char* pFunctionName, T tParam);

	template<typename TRet, typename T>
	TRet* CallUserFunction(const char* pFunctionName, T tParam);

	template<typename T1, typename T2>
	void CallVoidFunction(const char* pFunctionName, T1 tParam1, T2 tParam2);

	template<typename T1, typename T2>
	double CallNumberFunction(const char* pFunctionName, T1 tParam1,
			T2 tParam2);

	template<typename T1, typename T2>
	const char* CallStringFunction(const char* pFunctionName, T1 tParam1,
			T2 tParam2);

	template<typename TRet, typename T1, typename T2>
	TRet* CallUserFunction(const char* pFunctionName, T1 tParam1, T2 tParam2);
private:
	bool CallFunction(const char* pFunctionName, int nReturnNum);

	template<typename T>
	bool CallFunction(const char* pFunctionName, T tParam, int nReturnNum); //执行指定Lua文件中的函数//

	template<typename T1, typename T2>
	bool CallFunction(const char* pFunctionName, T1 tParam1, T2 tParam2,
			int nReturnNum = 0); //执行指定Lua文件中的函数//

	void PushValue(double lfValue);
//	void PushValue(const char* strValue);

	template<class T>
	void PushValue(T* pT);

	bool LoadFile(const char* pFileName);                           //加载指定的Lua文件//

	const char* LuaTraceBack();

private:
	lua_State* m_pState;   //这个是Lua的State对象指针，你可以一个lua文件对应一个。//
	lua_State* m_pBackState;	//用来重载的//

	std::vector<ToluaFunctionOpen*> m_vecToLuaFunctions;
};

template<class T>
inline void CLuaEngine::PushValue(T* pT)
{
	std::string strLuaType = "";
#ifdef _WIN32
	std::string strTypeName = typeid(*pT).name();
	bool bClass = false;
	char strLuaNameBuffer[128] = {0};
	//strcpy(strLuaNameBuffer, strTypeName.c_str());
	memcpy(strLuaNameBuffer, strTypeName.c_str(), strTypeName.size());
	for(char* strOuter = strtok(strLuaNameBuffer, " "); NULL != strOuter; strOuter = strtok(NULL, " "))
	{
		if(strcmp(strOuter, "class") == 0 || strcmp(strOuter, "struct") == 0)
		{
			bClass = true;
			continue;
		}
		if(strcmp(strOuter, "*") == 0)
		{
			assert(0);
		}
		if(bClass)
		{
			strLuaType = strOuter;
		}
	}
#else
	const char* strTypeName = typeid(*pT).name();
//linux下 格式为 nXXXX 指针为 Pnxxxx 若为指针类型 则是错的//
	assert(*strTypeName != 'P');
//数字开头 不知道为什么是最多2个//
	if (*strTypeName >= '0' && *strTypeName <= '9')
	{
		++strTypeName;
		if (*strTypeName >= '0' && *strTypeName <= '9')
		{
			++strTypeName;
		}
	}
	else
	{
		assert(0);
	}
	strLuaType = strTypeName;
#endif
	tolua_pushusertype(GetLuaState(), pT, strLuaType.c_str());
}

template<>
inline void CLuaEngine::PushValue<char>(char* strValue)
{
	tolua_pushstring(GetLuaState(), strValue);
}

template<typename T>
bool CLuaEngine::CallFunction(const char* pFunctionName, T tParam,
		int nReturnNum)
{
	lua_getglobal(GetLuaState(), pFunctionName);
	PushValue(tParam);
	int ret = lua_pcall(GetLuaState(), 1, nReturnNum, 0);
	if (ret != 0)
	{
		LogScreen("result : %d, error : %s", ret,
				lua_tostring(GetLuaState(), -1));
		lua_pop(GetLuaState(), 1);
		return false;
	}
	return true;
}

template<typename T1, typename T2>
bool CLuaEngine::CallFunction(const char* pFunctionName, T1 tParam1, T2 tParam2,
		int nReturnNum)
{
	lua_getglobal(GetLuaState(), pFunctionName);
	PushValue(tParam1);
	PushValue(tParam2);
	int ret = lua_pcall(GetLuaState(), 2, nReturnNum, 0);
	if (ret != 0)
	{
		LogScreen("result : %d, error : %s", ret,
				lua_tostring(GetLuaState(), -1));
		lua_pop(GetLuaState(), 1);
		return false;
	}
	return true;
}

template<typename TRet>
TRet* CLuaEngine::CallUserFunction(const char* pFunctionName)
{
	assert(CallFunction(pFunctionName, 1));
	assert(lua_isuserdata(GetLuaState(), -1) == 1);
	TRet* pRet = *(TRet**) (lua_touserdata(GetLuaState(), -1));
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return pRet;
}

template<typename T>
void CLuaEngine::CallVoidFunction(const char* pFunctionName, T tParam)
{
	assert(CallFunction(pFunctionName, tParam, 0));
	lua_settop(GetLuaState(), 0);
}

template<typename T>
double CLuaEngine::CallNumberFunction(const char* pFunctionName, T tParam)
{
	assert(CallFunction(pFunctionName, tParam, 1));
	assert(lua_isnumber(GetLuaState(), -1) == 1);
	double lfValue = lua_tonumber(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return lfValue;
}

template<typename T>
const char* CLuaEngine::CallStringFunction(const char* pFunctionName, T tParam)
{
	assert(CallFunction(pFunctionName, tParam, 1));
	assert(lua_isstring(GetLuaState(), -1) == 1);
	const char* strValue = lua_tostring(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return strValue;
}

template<typename TRet, typename T>
TRet* CLuaEngine::CallUserFunction(const char* pFunctionName, T tParam)
{
	assert(CallFunction(pFunctionName, tParam, 1));
	assert(lua_isuserdata(GetLuaState(), -1) == 1);
	TRet* pRet = *(TRet**) (lua_touserdata(GetLuaState(), -1));
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return pRet;
}

template<typename T1, typename T2>
void CLuaEngine::CallVoidFunction(const char* pFunctionName, T1 tParam1,
		T2 tParam2)
{
	assert(CallFunction(pFunctionName, tParam1, tParam2, 0));
	lua_settop(GetLuaState(), 0);
}

template<typename T1, typename T2>
double CLuaEngine::CallNumberFunction(const char* pFunctionName, T1 tParam1,
		T2 tParam2)
{
	assert(CallFunction(pFunctionName, tParam1, tParam2, 1));
	assert(lua_isnumber(GetLuaState(), -1) == 1);
	double lfValue = lua_tonumber(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return lfValue;
}

template<typename T1, typename T2>
const char* CLuaEngine::CallStringFunction(const char* pFunctionName,
		T1 tParam1, T2 tParam2)
{
	assert(CallFunction(pFunctionName, tParam1, tParam2, 1));
	assert(lua_isstring(GetLuaState(), -1) == 1);
	const char* strValue = lua_tostring(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return strValue;
}

template<typename TRet, typename T1, typename T2>
TRet* CLuaEngine::CallUserFunction(const char* pFunctionName, T1 tParam1,
		T2 tParam2)
{
	assert(CallFunction(pFunctionName, tParam1, tParam2, 1));
	assert(lua_isuserdata(GetLuaState(), -1) == 1);
	TRet* pRet = *(TRet**) (lua_touserdata(GetLuaState(), -1));
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return pRet;
}

#endif /* __LUA_ENGINE_H__ */
