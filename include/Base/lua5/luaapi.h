/* Reconstructed KingSoft luaapi.h.
 * The original DEV header was in no leak. This maps KingSoft's capitalized Lua API
 * (Lua_State, Lua_PushNumber, Lua_SetTable_CFunFromName, ...) onto the stock Lua 5.1
 * C API that libEngine_Lua5D.so already exports (lua_settable, lua_pushnumber, ...).
 * The Lua_SetTable_* helper bodies are ported verbatim from the SwordOnline engine's
 * LuaExtend.c ("扩展的 LuaAPI 函数"): guard lua_istable, push key, push value, settable.
 * NOTE: the table index passed to these helpers must be an ABSOLUTE (positive) stack
 * index, since two values are pushed before lua_settable. */
#ifndef _KG_LUAAPI_H_
#define _KG_LUAAPI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

/* KingSoft Lua pack/unpack buffer helpers — exported extern-C by libEngine_Lua5D.so.
 * All advance and return the buffer position. Signatures from call sites. */
#ifdef __cplusplus
extern "C" {
#endif
unsigned char* LuaPackup(lua_State* L, int nIndex, unsigned char* pby, size_t size);
unsigned char* LuaPackupNill(unsigned char* pby, size_t size);
unsigned char* LuaPackupBoolean(unsigned char* pby, size_t size, int bValue);
unsigned char* LuaPackupNumber(unsigned char* pby, size_t size, double dValue);
unsigned char* LuaPackupString(unsigned char* pby, size_t size, const char* szValue);
unsigned char* LuaUnpack(lua_State* L, unsigned char* pby, size_t size);
/* LuaPackage_* buffer builders — also extern-C exports of libEngine_Lua5D.so */
unsigned char* LuaPackage_NewTable(unsigned char* pby, size_t size);
unsigned char* LuaPackage_SetTable(unsigned char* pbyBegin, unsigned char* pbyEnd);
#ifdef __cplusplus
}
#endif

/* capitalized type aliases (SwordOnline LuaDef.h used #define; typedef is cleaner) */
typedef lua_State     Lua_State;
typedef lua_CFunction Lua_CFunction;

/* thin 1:1 aliases over stock Lua 5.1 (all provided by libEngine_Lua5D.so) */
#define Lua_GetTopIndex(L)       lua_gettop(L)
#define Lua_SetTop(L,i)          lua_settop((L),(i))
#define Lua_PushNil(L)           lua_pushnil(L)
#define Lua_PushNumber(L,n)      lua_pushnumber((L),(lua_Number)(n))
#define Lua_PushString(L,s)      lua_pushstring((L),(s))
#define Lua_PushBoolean(L,b)     lua_pushboolean((L),(b))
#define Lua_PushCFunction(L,f)   lua_pushcfunction((L),(f))
#define Lua_NewTable(L)          lua_newtable(L)
#define Lua_SetTable(L,i)        lua_settable((L),(i))
#define Lua_GetTable(L,i)        lua_gettable((L),(i))
#define Lua_IsTable(L,i)         lua_istable((L),(i))
#define Lua_ValueToNumber(L,i)   lua_tonumber((L),(i))
#define Lua_ValueToString(L,i)   lua_tostring((L),(i))
#define Lua_ValueToBoolean(L,i)  lua_toboolean((L),(i))
#define Lua_ValueToUserData(L,i) lua_touserdata((L),(i))
#define Lua_Pop(L,n)             lua_pop((L),(n))
#define Lua_SetTopIndex(L,i)     lua_settop((L),(i))
#define Lua_Next(L,i)            lua_next((L),(i))
#define Lua_IsNumber(L,i)        lua_isnumber((L),(i))
#define Lua_IsString(L,i)        lua_isstring((L),(i))
#define Lua_IsBoolean(L,i)       lua_isboolean((L),(i))
#define Lua_IsNil(L,i)           lua_isnil((L),(i))
#define Lua_IsFunction(L,i)      lua_isfunction((L),(i))

/* Lua_SetTable_* family — bodies ported from LuaExtend.c */
static inline int Lua_SetTable_IntFromId(Lua_State* L, int nIndex, int Id, int nNumber) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushnumber(L, (lua_Number)Id); lua_pushnumber(L, (lua_Number)nNumber);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_IntFromName(Lua_State* L, int nIndex, const char* szName, int nNumber) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushstring(L, szName); lua_pushnumber(L, (lua_Number)nNumber);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_DoubleFromId(Lua_State* L, int nIndex, int Id, double dNumber) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushnumber(L, (lua_Number)Id); lua_pushnumber(L, (lua_Number)dNumber);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_DoubleFromName(Lua_State* L, int nIndex, const char* szName, double dNumber) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushstring(L, szName); lua_pushnumber(L, (lua_Number)dNumber);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_StringFromId(Lua_State* L, int nIndex, int Id, const char* szValue) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushnumber(L, (lua_Number)Id); lua_pushstring(L, szValue);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_StringFromName(Lua_State* L, int nIndex, const char* szName, const char* szValue) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushstring(L, szName); lua_pushstring(L, szValue);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_CFunFromId(Lua_State* L, int nIndex, int nId, Lua_CFunction CFun) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushnumber(L, (lua_Number)nId); lua_pushcfunction(L, CFun);
    lua_settable(L, nIndex); return 1;
}
static inline int Lua_SetTable_CFunFromName(Lua_State* L, int nIndex, const char* szName, Lua_CFunction CFun) {
    if (!lua_istable(L, nIndex)) return 0;
    lua_pushstring(L, szName); lua_pushcfunction(L, CFun);
    lua_settable(L, nIndex); return 1;
}

#endif /* _KG_LUAAPI_H_ */
