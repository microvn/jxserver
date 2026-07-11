#ifndef _KEXTERIOR_H_
#define _KEXTERIOR_H_

//////////////////////////////////////////////////////////////////////////
// KExterior -- singleton config manager for the exterior (wai-guan) system.
// Loads 4 tables from settings/Exterior/ and answers config queries + (later)
// owns the buy chain. Ported from SO3GameServer v246; see
// linux-build/docs/exterior_port/WORKLOG.md [PORT-2].
// Embedded as a member of KWorldSettings (binary: world+0x92620, member order
// info/suit/index/shop). Recompiled -> member offsets are compiler-chosen.
//////////////////////////////////////////////////////////////////////////

#include <map>
#include "KExteriorDef.h"

class KPlayer;

// Suit config row (ExteriorSuitInfo.tab). Key = SuitID (map key, not stored). 7 dwords.
struct KEXTERIOR_SUIT_INFO
{
    DWORD   nChest;          // +0x00
    DWORD   nHelm;           // +0x04
    DWORD   nWaist;          // +0x08
    DWORD   nBoots;          // +0x0c
    DWORD   nBangle;         // +0x10
    int     nAchievementID;  // +0x14  default -1; validated [0, MAX_EXTERIOR_ACHIEVEMENT_ID)
    BOOL    bNeedPermanent;  // +0x18  stored = (column > 0)
};                           // sizeof = 0x1c

// Shop price row (Exteriorshop.tab). Key = ID (map key, not stored). 10 dwords.
struct KEXTERIOR_SHOP_PRICE
{
    int     nPrice[ettTotal][epctTotal];    // +0x00  [time][pay]; [i][epctMoney] x10000
    DWORD   nLimitType;                      // +0x24
};                                           // sizeof = 0x28

// Reverse-lookup key (ExteriorBuy.tab): appearance -> exterior ID. 4 dwords.
struct KEXTERIOR_INDEX_KEY
{
    DWORD   nSubType;        // +0x00
    DWORD   nRepresentID;    // +0x04
    DWORD   nColorID;        // +0x08
    DWORD   nForceID;        // +0x0c

    bool operator<(const KEXTERIOR_INDEX_KEY& r) const
    {
        if (nSubType     != r.nSubType)     return nSubType     < r.nSubType;
        if (nRepresentID != r.nRepresentID) return nRepresentID < r.nRepresentID;
        if (nColorID     != r.nColorID)     return nColorID     < r.nColorID;
        return nForceID < r.nForceID;
    }
};                           // sizeof = 0x10

class KExterior
{
public:
    BOOL Init();
    void UnInit();

    // Grant an exterior to a player (the LuaAddExterior path: validates the
    // price combo exists, computes the expire from time-type, adds to the box).
    // No currency is deducted -- the money/coin buy chain (shop) is separate.
    BOOL AddExterior(KPlayer* pPlayer, DWORD dwID, int nTimeType, int nPayType, int nBuySource);

    KEXTERIOR_INFO*       GetExteriorInfo(DWORD dwID);
    KEXTERIOR_SUIT_INFO*  GetExteriorSuitInfo(DWORD dwSuitID);
    KEXTERIOR_SHOP_PRICE* GetExteriorShopPrice(DWORD dwID);
    DWORD                 GetExteriorIndex(DWORD nSubType, DWORD nRepresentID, DWORD nColorID, DWORD nForceID);

private:
    BOOL LoadExteriorInfoTable();
    BOOL LoadExteriorSuitInfoTable();
    BOOL LoadExteriorIndexInfoTable();
    BOOL LoadExteriorShopPriceTable();

private:
    typedef std::map<DWORD, KEXTERIOR_INFO>          KEXTERIOR_INFO_MAP;
    typedef std::map<DWORD, KEXTERIOR_SUIT_INFO>     KEXTERIOR_SUIT_MAP;
    typedef std::map<KEXTERIOR_INDEX_KEY, DWORD>     KEXTERIOR_INDEX_MAP;
    typedef std::map<DWORD, KEXTERIOR_SHOP_PRICE>    KEXTERIOR_SHOP_MAP;

    KEXTERIOR_INFO_MAP    m_ExteriorInfoMap;    // ExteriorInfo.tab
    KEXTERIOR_SUIT_MAP    m_ExteriorSuitMap;    // ExteriorSuitInfo.tab
    KEXTERIOR_INDEX_MAP   m_ExteriorIndexMap;   // ExteriorBuy.tab (reverse lookup)
    KEXTERIOR_SHOP_MAP    m_ExteriorShopMap;    // Exteriorshop.tab
};

#endif  // _KEXTERIOR_H_
