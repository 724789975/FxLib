/*
** Lua binding: LuaMeta
** Generated automatically by tolua++-1.0.6 on 06/02/16 23:42:35.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int tolua_LuaMeta_open (lua_State* tolua_S);

#include "fxmeta.h"
#include "ifnet.h"
#include "SocketSession.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_SOCKET (lua_State* tolua_S)
{
 SOCKET* self = (SOCKET*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_IFxNet (lua_State* tolua_S)
{
 IFxNet* self = (IFxNet*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"IFxSessionFactory");
 tolua_usertype(tolua_S,"FxSession");
 tolua_usertype(tolua_S,"UINT16");
 tolua_usertype(tolua_S,"SOCKET");
 tolua_usertype(tolua_S,"UINT32");
 tolua_usertype(tolua_S,"IFxListenSocket");
 tolua_usertype(tolua_S,"IFxNet");
}

/* method: delete of class  IFxNet */
static int tolua_LuaMeta_IFxNet_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'",NULL);
#endif
 delete self;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}

/* method: Init of class  IFxNet */
static int tolua_LuaMeta_IFxNet_Init00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Init'",NULL);
#endif
 {
  bool tolua_ret = (bool)  self->Init();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Init'.",&tolua_err);
 return 0;
#endif
}

/* method: Run of class  IFxNet */
static int tolua_LuaMeta_IFxNet_Run00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"UINT32",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
  UINT32 dwCount = *((UINT32*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Run'",NULL);
#endif
 {
  bool tolua_ret = (bool)  self->Run(dwCount);
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Run'.",&tolua_err);
 return 0;
#endif
}

/* method: Release of class  IFxNet */
static int tolua_LuaMeta_IFxNet_Release00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Release'",NULL);
#endif
 {
  self->Release();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Release'.",&tolua_err);
 return 0;
#endif
}

/* method: Connect of class  IFxNet */
static int tolua_LuaMeta_IFxNet_Connect00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"FxSession",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,3,"UINT32",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,4,"UINT16",0,&tolua_err) ||
 !tolua_isboolean(tolua_S,5,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
  FxSession* poSession = ((FxSession*)  tolua_tousertype(tolua_S,2,0));
  UINT32 dwIP = *((UINT32*)  tolua_tousertype(tolua_S,3,0));
  UINT16 wPort = *((UINT16*)  tolua_tousertype(tolua_S,4,0));
  bool bReconnect = ((bool)  tolua_toboolean(tolua_S,5,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Connect'",NULL);
#endif
 {
  SOCKET tolua_ret = (SOCKET)  self->Connect(poSession,dwIP,wPort,bReconnect);
 {
#ifdef __cplusplus
 void* tolua_obj = new SOCKET(tolua_ret);
 tolua_pushusertype_and_takeownership(tolua_S,tolua_obj,"SOCKET");
#else
 void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(SOCKET));
 tolua_pushusertype_and_takeownership(tolua_S,tolua_obj,"SOCKET");
#endif
 }
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Connect'.",&tolua_err);
 return 0;
#endif
}

/* method: Listen of class  IFxNet */
static int tolua_LuaMeta_IFxNet_Listen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"IFxNet",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"IFxSessionFactory",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,3,"UINT32",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,4,"UINT32",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,5,"UINT16",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  IFxNet* self = (IFxNet*)  tolua_tousertype(tolua_S,1,0);
  IFxSessionFactory* pSessionFactory = ((IFxSessionFactory*)  tolua_tousertype(tolua_S,2,0));
  UINT32 dwListenId = *((UINT32*)  tolua_tousertype(tolua_S,3,0));
  UINT32 dwIP = *((UINT32*)  tolua_tousertype(tolua_S,4,0));
  UINT16 dwPort = *((UINT16*)  tolua_tousertype(tolua_S,5,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Listen'",NULL);
#endif
 {
  IFxListenSocket* tolua_ret = (IFxListenSocket*)  self->Listen(pSessionFactory,dwListenId,dwIP,dwPort);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"IFxListenSocket");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Listen'.",&tolua_err);
 return 0;
#endif
}

/* function: FxNetGetModule */
static int tolua_LuaMeta_FxNetGetModule00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  IFxNet* tolua_ret = (IFxNet*)  FxNetGetModule();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"IFxNet");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FxNetGetModule'.",&tolua_err);
 return 0;
#endif
}

/* function: SetPortC */
static int tolua_LuaMeta_SetPortC00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  unsigned int dwPort = ((unsigned int)  tolua_tonumber(tolua_S,1,0));
 {
  SetPortC(dwPort);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetPortC'.",&tolua_err);
 return 0;
#endif
}

/* Open function */
TOLUA_API int tolua_LuaMeta_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"IFxNet","IFxNet","",tolua_collect_IFxNet);
#else
 tolua_cclass(tolua_S,"IFxNet","IFxNet","",NULL);
#endif
 tolua_beginmodule(tolua_S,"IFxNet");
 tolua_function(tolua_S,"delete",tolua_LuaMeta_IFxNet_delete00);
 tolua_function(tolua_S,"Init",tolua_LuaMeta_IFxNet_Init00);
 tolua_function(tolua_S,"Run",tolua_LuaMeta_IFxNet_Run00);
 tolua_function(tolua_S,"Release",tolua_LuaMeta_IFxNet_Release00);
 tolua_function(tolua_S,"Connect",tolua_LuaMeta_IFxNet_Connect00);
 tolua_function(tolua_S,"Listen",tolua_LuaMeta_IFxNet_Listen00);
 tolua_endmodule(tolua_S);
 tolua_function(tolua_S,"FxNetGetModule",tolua_LuaMeta_FxNetGetModule00);
 tolua_function(tolua_S,"SetPortC",tolua_LuaMeta_SetPortC00);
 tolua_endmodule(tolua_S);
 return 1;
}
