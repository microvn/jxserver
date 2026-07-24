#ifndef _KHAIRSHOP_DEF_H_
#define _KHAIRSHOP_DEF_H_

//////////////////////////////////////////////////////////////////////////
// KHairShop / KHairBox shared enums + leaf structs (发型/hair).
// Ported from SO3GameServer v2.5 (v246). Every offset/size below is pinned
// from the DWARF binary jx3_dwarf/SO3GameServerD via llvm-dwarfdump --name -c
// (NOT guessed from pseudo-C) -- see linux-build/docs/hair_port/WORKLOG.md
// [RE-1..3]. Layout is authoritative for the DB round-trip so DB structs are
// #pragma pack(1) and sizes are verified with a negative-array guard.
// Original header: Source/Common/SO3World/Src/KHairShop.h (per DW_AT_decl_file).
//////////////////////////////////////////////////////////////////////////

// Hair slot type -- Find/_Add/ChangeHair all assert uType < hsTotal (==2).
// hsFace/hsHair index both m_HairList[2] and the applied appearance short[2].
enum HAIR_SLOT_TYPE
{
    hsFace  = 0,    // face style slot
    hsHair  = 1,    // hair style slot
    hsTotal = 2,
};

// ---- Capacity constants (pinned) ----
#define MAX_HAIR_LIST_SIZE      1024    // in-memory per-slot list cap (_Add)
#define MAX_HAIR_FREE_COUNT     60000   // free-change counter cap (AddHairFreeCount)

// ---- Config-table rows (settings/HairShop/*.tab), stored in KHairShop maps.
//      Keys are map keys, not struct members. Sizes from DWARF (KHairShop.h). ----

// HeadIndex.tab value. Key = HeadID (DWORD, map key). KHairShop.h:8, size 0x0c.
struct KHAIR_INDEX_INFO
{
    DWORD   dwHeadformID;   // +0x00
    DWORD   dwBangID;       // +0x04
    DWORD   dwPlaitID;      // +0x08
};                          // sizeof = 0x0c

// HairPrice.tab value. Key = KHAIR_PRICE_KEY. KHairShop.h:33, size 0x0c.
struct KHAIR_PRICE_INFO
{
    int     nPrice;         // +0x00  in-game money price (loader asserts == 0)
    int     nCoin;          // +0x04  coin (点券) price; loader asserts >= 0
    int     nNeedFreeCount; // +0x08  free-change tokens required
};                          // sizeof = 0x0c

// HairPrice.tab key: (role type, slot type, hair id). KHairShop.h:15, size 0x0c.
struct KHAIR_PRICE_KEY
{
    int     nRoleType;      // +0x00
    int     nType;          // +0x04  hair slot type (hsFace/hsHair)
    DWORD   dwID;           // +0x08

    bool operator<(const KHAIR_PRICE_KEY& r) const
    {
        if (nRoleType != r.nRoleType) return nRoleType < r.nRoleType;
        if (nType     != r.nType)     return nType     < r.nType;
        return dwID < r.dwID;
    }
};                          // sizeof = 0x0c

// Buy params (KHairShop.h:40 / :47). Used by the buy chain (deferred, slice #5).
// dwID[hsFace], dwID[hsHair]; bUseFreeCount parallels. Pinned sizes 0x10 / 0x18.
struct KHAIRSHOP_BUY_PARAM
{
    DWORD   dwID[hsTotal];          // +0x00
    BOOL    bUseFreeCount[hsTotal]; // +0x08
};                                  // sizeof = 0x10

struct KHAIRSHOP_COIN_BUY_PARAM
{
    int     nPrice;                 // +0x00
    int     nFreeCount;             // +0x04
    DWORD   dwID[hsTotal];          // +0x08
    BOOL    bUseFreeCount[hsTotal]; // +0x10
};                                  // sizeof = 0x18

//////////////////////////////////////////////////////////////////////////
// DB serialization struct -- byte layout must match v246 exactly.
// KHAIR_DB_DATA::KHAIR_INFO = 8B {WORD wID; BYTE reserved[6]} (KRoleDBDataDef.h
// in v246). Per-slot block = [WORD blockSize][WORD wCount][item x wCount];
// blockSize = 2 + wCount*8. Both slot blocks always written, then a trailing
// [WORD freeCount]. (slice #3 -- Save/Load.)
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)

struct KHAIR_DB_DATA
{
    struct KHAIR_INFO               // 8B DB item
    {
        WORD    wID;                // +0x00  (hair id, must be <= 65535)
        BYTE    byReserved[6];      // +0x02 -> pad to 0x08
    };                              // sizeof = 0x08
    WORD        wCount;             // +0x00  (per-slot item count)
    KHAIR_INFO  items[0];           // +0x02
};                                  // header sizeof = 2

#pragma pack()

#endif  // _KHAIRSHOP_DEF_H_
