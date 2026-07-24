#include "stdafx.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KHairShop.h"

#ifdef _SERVER

//////////////////////////////////////////////////////////////////////////
// KHairShop Lua bindings. Self = the global KHairShop singleton via
// Luna<KHairShop> (see KScriptCenter register + KBaseFuncList::LuaGetHairShop).
// Routing pinned from v246 (WORKLOG [RE-2]). Buy (LuaBuyHair) is deferred.
//////////////////////////////////////////////////////////////////////////

int KHairShop::LuaOpenHairShop(Lua_State* L)
{
    int nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);
    m_bCloseHairShop = false;
Exit0:
    return 0;
}

int KHairShop::LuaCloseHairShop(Lua_State* L)
{
    int nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);
    m_bCloseHairShop = true;
Exit0:
    return 0;
}

// (playerID, hairID, faceID, bFreeHair, bFreeFace) -> grant + apply.
// bFree* are part of the (deferred) buy accounting; the grant path ignores them.
int KHairShop::LuaChangePlayerHair(Lua_State* L)
{
    int      bResult   = false;
    int      nTopIndex = 0;
    DWORD    dwPlayerID = 0;
    DWORD    dwHairID   = 0;
    DWORD    dwFaceID   = 0;
    KPlayer* pPlayer    = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 5);

    dwPlayerID = (DWORD)Lua_ValueToNumber(L, 1);
    dwHairID   = (DWORD)Lua_ValueToNumber(L, 2);
    dwFaceID   = (DWORD)Lua_ValueToNumber(L, 3);

    pPlayer = g_pSO3World->m_PlayerSet.GetObj(dwPlayerID);
    KG_PROCESS_ERROR(pPlayer);

    bResult = ChangePlayerHair(pPlayer, dwHairID, dwFaceID);
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

// (headID) -> headformID, bangID, plaitID
int KHairShop::LuaGetHairIndex(Lua_State* L)
{
    int               nResult   = 0;
    int               nTopIndex = 0;
    DWORD             dwHeadID  = 0;
    KHAIR_INDEX_INFO* pInfo     = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);

    dwHeadID = (DWORD)Lua_ValueToNumber(L, 1);
    pInfo = GetHairIndexInfo(dwHeadID);
    KG_PROCESS_ERROR(pInfo);

    Lua_PushNumber(L, (double)pInfo->dwHeadformID);
    Lua_PushNumber(L, (double)pInfo->dwBangID);
    Lua_PushNumber(L, (double)pInfo->dwPlaitID);
    nResult = 3;
Exit0:
    return nResult;
}

// (roleType, type, id) -> price, coin, needFreeCount
int KHairShop::LuaGetHairPrice(Lua_State* L)
{
    int               nResult   = 0;
    int               nTopIndex = 0;
    int               nRoleType = 0;
    int               nType     = 0;
    DWORD             dwID      = 0;
    KHAIR_PRICE_INFO* pInfo     = NULL;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 3);

    nRoleType = (int)Lua_ValueToNumber(L, 1);
    nType     = (int)Lua_ValueToNumber(L, 2);
    dwID      = (DWORD)Lua_ValueToNumber(L, 3);

    pInfo = GetPriceInfo(nRoleType, nType, dwID);
    KG_PROCESS_ERROR(pInfo);

    Lua_PushNumber(L, (double)pInfo->nPrice);
    Lua_PushNumber(L, (double)pInfo->nCoin);
    Lua_PushNumber(L, (double)pInfo->nNeedFreeCount);
    nResult = 3;
Exit0:
    return nResult;
}

DEFINE_LUA_CLASS_BEGIN(KHairShop)
    REGISTER_LUA_FUNC(KHairShop, OpenHairShop)
    REGISTER_LUA_FUNC(KHairShop, CloseHairShop)
    REGISTER_LUA_FUNC(KHairShop, ChangePlayerHair)
    REGISTER_LUA_FUNC(KHairShop, GetHairIndex)
    REGISTER_LUA_FUNC(KHairShop, GetHairPrice)
DEFINE_LUA_CLASS_END(KHairShop)

#endif // _SERVER
