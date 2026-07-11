#ifndef _KHAIRSHOP_H_
#define _KHAIRSHOP_H_

//////////////////////////////////////////////////////////////////////////
// KHairShop -- singleton config manager for the hair (fa-xing) system.
// Loads 2 tables from settings/HairShop/ and answers config queries; later
// owns the grant path (AddHair) + buy chain (deferred). Ported from
// SO3GameServer v246; see linux-build/docs/hair_port/PORT_DESIGN.md.
// Lua binding is Luna<KHairShop> (self = the global singleton), NOT a KPlayer
// method -- see WORKLOG [RE-2] and docs/../TODO.md for why the shape matters.
// Recompiled -> member offsets are compiler-chosen (binary layout is a check).
//////////////////////////////////////////////////////////////////////////

#include <map>
#include "KHairShopDef.h"
#ifdef _SERVER
#include "Luna.h"
#endif

class KPlayer;

class KHairShop
{
public:
    BOOL Init();
    void UnInit();

    KHAIR_INDEX_INFO* GetHairIndexInfo(DWORD dwHeadID);
    KHAIR_PRICE_INFO* GetPriceInfo(int nRoleType, int nType, DWORD dwID);

    // Grant path (money/coin-free): give the hair to the box then apply it.
    // == LuaChangePlayerHair core (v246); the buy chain is deferred (needs coin).
    BOOL AddHair(KPlayer* pPlayer, int nType, DWORD dwID);
    BOOL ChangePlayerHair(KPlayer* pPlayer, DWORD dwHairID, DWORD dwFaceID);

#ifdef _SERVER
public:
    // Lua bindings -- self = the global KHairShop singleton (Luna<KHairShop>),
    // reached from script via GetHairShop(). NOT a KPlayer method.
    int LuaOpenHairShop(Lua_State* L);
    int LuaCloseHairShop(Lua_State* L);
    int LuaChangePlayerHair(Lua_State* L);
    int LuaGetHairIndex(Lua_State* L);
    int LuaGetHairPrice(Lua_State* L);

    DECLARE_LUA_CLASS(KHairShop);
#endif

private:
    BOOL LoadHairIndexTable();
    BOOL LoadHairPriceTable();

private:
    typedef std::map<DWORD, KHAIR_INDEX_INFO>            KHAIR_INDEX_MAP;
    typedef std::map<KHAIR_PRICE_KEY, KHAIR_PRICE_INFO>  KHAIR_PRICE_MAP;

    BOOL            m_bCloseHairShop;   // +0x00 in v246; close flag
    KHAIR_INDEX_MAP m_HairIndexInfoMap; // HeadIndex.tab  (HeadID -> {headform,bang,plait})
    KHAIR_PRICE_MAP m_HairPriceInfoMap; // HairPrice.tab  ({role,type,id} -> price)
};

#endif  // _KHAIRSHOP_H_
