#include "lua_engine.h"
#include "string.h"
#include <string>

IMPLEMENT_SINGLETON(CLuaEngine)

CLuaEngine::CLuaEngine()
{
	m_pState = NULL;
	m_pBackState = NULL;
// TODO Auto-generated constructor stub
}

CLuaEngine::~CLuaEngine()
{
// TODO Auto-generated destructor stub
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
	luaopen_base(m_pBackState);   // ����Lua������//
	luaL_openlibs(m_pBackState);  // ����Luaͨ����չ��//

	for (int i = 0; i < (int) m_vecToLuaFunctions.size(); ++i)
	{
		(*m_vecToLuaFunctions[i])(m_pBackState);
	}

	char* strExePath = GetExePath();

	char strScriptPath[256];
#ifdef WIN32
	sprintf(strScriptPath, "%s%s%s%s", strExePath, "\\", WORK_PATH, "\\");
#else
	sprintf(strScriptPath, "%s%s%s", strExePath, "/", WORK_PATH);
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

void CLuaEngine::CallVoidFunction(const char* pFunctionName)
{
	assert(CallFunction(pFunctionName, 0));
}

double CLuaEngine::CallNumberFunction(const char* pFunctionName)
{
	assert(CallFunction(pFunctionName, 1));
	assert(lua_isnumber(GetLuaState(), -1) == 1);
	double lfValue = lua_tonumber(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	lua_settop(GetLuaState(), 0);
	return lfValue;
}
const char* CLuaEngine::CallStringFunction(const char* pFunctionName)
{
	assert(CallFunction(pFunctionName, 1));
	assert(lua_isstring(GetLuaState(), -1) == 1);
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
		LogScreen("result : %d, error : %s", ret,
				lua_tostring(GetLuaState(), -1));
		lua_pop(GetLuaState(), 1);
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
		LogScreen("lua_State is NULL");
		return false;
	}

	char pc[512] = { 0 };  //�㹻��//
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
		LogScreen("lua_loadfile : %s, result : %d, err : %s", pFileName, nRet,
				lua_tostring(m_pBackState, -1));
		return false;
	}

	return true;
}

const char* CLuaEngine::LuaTraceBack()
{
	// ��ӡlua����ջ��ʼ//
	lua_getglobal(GetLuaState(), "debug");
	lua_getfield(GetLuaState(), -1, "traceback");
	int iError = lua_pcall(GetLuaState(),    //VMachine
			0,    //Argument Count
			1,    //Return Value Count
			0);
	const char* sz = lua_tostring(GetLuaState(), -1);
	lua_pop(GetLuaState(), 1);
	return sz;
}

