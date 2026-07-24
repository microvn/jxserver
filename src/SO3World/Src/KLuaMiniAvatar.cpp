#include "stdafx.h"
#include "KSO3World.h"
#include "KPlayer.h"
#include "KMiniAvatar.h"

#ifdef _SERVER

//////////////////////////////////////////////////////////////////////////
// KMiniAvatar Lua bindings. Self = the per-player box (Luna<KMiniAvatar>),
// reached via KPlayer:GetMiniAvatarMgr() (see KLuaPlayer.cpp). Buy/wear go
// through KPlayer:SetMiniAvatar. Routing pinned from v246 (WORKLOG [RE-2]).
//////////////////////////////////////////////////////////////////////////

int KMiniAvatar::LuaAcquireMiniAvatar(Lua_State* L)
{
    int   bResult   = false;
    int   nTopIndex = 0;
    DWORD dwID      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);
    dwID = (DWORD)Lua_ValueToNumber(L, 1);

    bResult = Acquire(dwID);
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KMiniAvatar::LuaIsMiniAvatarAcquired(Lua_State* L)
{
    int   bResult   = false;
    int   nTopIndex = 0;
    DWORD dwID      = 0;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 1);
    dwID = (DWORD)Lua_ValueToNumber(L, 1);

    bResult = IsAcquired(dwID);
Exit0:
    Lua_PushBoolean(L, bResult);
    return 1;
}

int KMiniAvatar::LuaGetMiniAvatarCount(Lua_State* L)
{
    int nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);
    Lua_PushNumber(L, (double)GetCount());
Exit0:
    return 1;
}

// push each acquired id as a return value (script: local a,b,... = mgr:GetAllMiniAvatar()).
int KMiniAvatar::LuaGetAllMiniAvatar(Lua_State* L)
{
    int nResult = 0;
    int nTopIndex = 0;
    std::set<DWORD>::iterator it;

    nTopIndex = Lua_GetTopIndex(L);
    KGLOG_PROCESS_ERROR(nTopIndex == 0);
    for (it = m_MiniAvatarSet.begin(); it != m_MiniAvatarSet.end(); ++it)
    {
        Lua_PushNumber(L, (double)(*it));
        nResult++;
    }
Exit0:
    return nResult;
}

DEFINE_LUA_CLASS_BEGIN(KMiniAvatar)
    REGISTER_LUA_FUNC(KMiniAvatar, AcquireMiniAvatar)
    REGISTER_LUA_FUNC(KMiniAvatar, IsMiniAvatarAcquired)
    REGISTER_LUA_FUNC(KMiniAvatar, GetMiniAvatarCount)
    REGISTER_LUA_FUNC(KMiniAvatar, GetAllMiniAvatar)
DEFINE_LUA_CLASS_END(KMiniAvatar)

#endif // _SERVER
