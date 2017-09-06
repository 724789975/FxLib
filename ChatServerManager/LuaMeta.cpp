/*
** Lua binding: LuaMeta
** Generated automatically by tolua++-1.0.6 on 09/07/17 00:20:13.
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
#include "LuaMeta.h"
#include "utility.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

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
 tolua_usertype(tolua_S,"UINT32");
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
 tolua_endmodule(tolua_S);
 tolua_function(tolua_S,"FxNetGetModule",tolua_LuaMeta_FxNetGetModule00);
 tolua_endmodule(tolua_S);
 return 1;
}
