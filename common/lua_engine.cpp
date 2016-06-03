#include "lua_engine.h"
#include "string.h"
#include <string>

IMPLEMENT_SINGLETON(CLuaEngine)

CLuaEngine::CLuaEngine()
{
	m_pState = NULL;
	m_pBackState = NULL;
}

CLuaEngine::~CLuaEngine()
{
}

bool CLuaEngine::Init(std::vector<ToluaFunctionOpen*>& vecToLuaFunctions)
{
	m_vecToLuaFunctions = vecToLuaFunctions;
	return true;
}

bool CLuaEngine::Reload()
{
	if (m_pBackState != NULL)
	{
		lua_close(m_pBackState);
		m_pBackState = NULL;
	}

	if ((m_pBackState = lua_open()) == NULL)
	{
		return false;
	}
	luaopen_base(m_pBackState);   // 加载Lua基本库//
	luaL_openlibs(m_pBackState);  // 加载Lua通用扩展库//

	for (int i = 0; i < (int) m_vecToLuaFunctions.size(); ++i)
	{
		(*m_vecToLuaFunctions[i])(m_pBackState);
	}

	char* strExePath = GetExePath();

	char strScriptPath[256];
#ifdef WIN32
	sprintf(strScriptPath, "%s%s%s%s", strExePath, "\\", WORK_PATH, "\\");
#else
	sprintf(strScriptPath, "%s%s%s%s", strExePath, "/", WORK_PATH, "/");

	char strLuaPath[256] = {0};
	sprintf(strLuaPath, "local p = '%s'\n"
			"local m_package_path = package.path\n"
			"package.path = string.format('%%s;%%s?.lua;%%s?/init.lua', m_package_path, p, p)\n",
			strScriptPath);
	luaL_loadstring(m_pBackState, strLuaPath);
	int ret = lua_pcall(m_pBackState, 0, 0, 0);
	if (ret != 0)
	{
		printf("result : %d, error : %s", ret, lua_tostring(m_pBackState, -1));
		lua_pop(m_pBackState, 1);
		return false;
	}
	lua_settop(m_pBackState, 0);
#endif // WIN32
	ListDir(strScriptPath, this);

	lua_settop(m_pBackState, 0);

	lua_State* pTemp = m_pState;
	m_pState = m_pBackState;
	m_pBackState = pTemp;

	if (m_pBackState != NULL)
	{
		lua_close(m_pBackState);
		m_pBackState = NULL;
	}

	return true;
}

void CLuaEngine::Close()
{
	if (m_pBackState != NULL)
	{
		lua_close(m_pBackState);
		m_pBackState = NULL;
	}

	if (m_pState != NULL)
	{
		lua_close(m_pState);
		m_pState = NULL;
	}
}

lua_State* CLuaEngine::GetLuaState()
{
	return m_pState;
}

bool CLuaEngine::CommandLineFunction(char** ppstrArg, unsigned int dwArgNum)
{
	lua_newtable(GetLuaState());
	for (unsigned int i = 0; i < dwArgNum; ++i)
	{
		lua_pushnumber(GetLuaState(), i);
		lua_pushstring(GetLuaState(), ppstrArg[i]);
		lua_settable(GetLuaState(), -3);
	}
	lua_setglobal(GetLuaState(), "tableCommandLine");
	lua_settop(GetLuaState(), 0);

	lua_getglobal(GetLuaState(), "GetoptLong");
	int nRet = lua_pcall(GetLuaState(), 0, 1, 0);
	if (nRet != 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "result : %d, error : %s", nRet, lua_tostring(GetLuaState(), -1));
		//printf("result : %d, error : %s\n", nRet, lua_tostring(GetLuaState(), -1));
		lua_pop(GetLuaState(), 1);
		lua_settop(GetLuaState(), 0);
		return false;
	}
	bool bRet = lua_toboolean(GetLuaState(), -1) > 0;
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return bRet;
}

void CLuaEngine::CallVoidFunction(const char* pFunctionName)
{
	Assert(CallFunction(pFunctionName, 0));
}

double CLuaEngine::CallNumberFunction(const char* pFunctionName)
{
	Assert(CallFunction(pFunctionName, 1));
	Assert(lua_isnumber(GetLuaState(), -1) == 1);
	double lfValue = lua_tonumber(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return lfValue;
}
const char* CLuaEngine::CallStringFunction(const char* pFunctionName)
{
	Assert(CallFunction(pFunctionName, 1));
	Assert(lua_isstring(GetLuaState(), -1) == 1);
	const char* strValue = lua_tostring(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return strValue;
}

bool CLuaEngine::CallFunction(const char* pFunctionName,
		int nReturnNum /* = 0 */)
{
	lua_getglobal(GetLuaState(), pFunctionName);
	int ret = lua_pcall(GetLuaState(), 0, nReturnNum, 0);
	if (ret != 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "result : %d, error : %s", ret, lua_tostring(GetLuaState(), -1));
		lua_pop(GetLuaState(), 1);
		printf("false\n");
		return false;
	}
	return true;
}

void CLuaEngine::PushValue(double lfValue)
{
	tolua_pushnumber(GetLuaState(), lfValue);
}

//void CLuaEngine::PushValue(const char* strValue)
//{
//	tolua_pushstring(GetLuaState(), strValue);
//}

bool CLuaEngine::LoadFile(const char* pFileName)
{
	int nRet = 0;
	if (NULL == m_pBackState)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "%s", "lua_State is NULL");
		return false;
	}

	char pc[512] = { 0 };  //足够长//
	//strcpy(pc, pFileName);
	memcpy(pc, pFileName, strlen(pFileName));

#ifdef WIN32
	char* strFileExt = strrchr(pc, '.');
#else
	char* strFileExt = rindex(pc, '.');
#endif // WIN32


	if (strcmp(strFileExt, ".lua") != 0)
	{
		return false;
	}

	nRet = luaL_dofile(m_pBackState, pFileName);
	if (nRet != 0)
	{
		LogFun(LT_Screen | LT_File, LogLv_Error, "lua_loadfile : %s, result : %d, err : %s", pFileName, nRet, lua_tostring(m_pBackState, -1));
		return false;
	}

	return true;
}

const char* CLuaEngine::LuaTraceBack()
{
	// 打印lua调用栈开始//
	lua_getglobal(GetLuaState(), "debug");
	lua_getfield(GetLuaState(), -1, "traceback");
	lua_pcall(GetLuaState(),    //VMachine
			0,    //Argument Count
			1,    //Return Value Count
			0);
	const char* sz = lua_tostring(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	return sz;
}

