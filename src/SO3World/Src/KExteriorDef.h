#ifndef _KEXTERIOR_DEF_H_
#define _KEXTERIOR_DEF_H_

//////////////////////////////////////////////////////////////////////////
// KExterior / KExteriorBox shared enums + leaf structs.
// Ported from SO3GameServer v246 (build 4503). Every offset/size/enum
// value below is pinned from the binary via Ghidra -- see
// linux-build/docs/exterior_port/WORKLOG.md [RE-*] and [PORT-1].
// Layout is authoritative for DB round-trip + client packet, so DB structs
// are #pragma pack(1) and sizes are verified with static asserts.
//////////////////////////////////////////////////////////////////////////

// ---- Enums (values pinned from binary; ".*Invalid"/".*Total" sentinels
//      come from the assert strings, e.g. "nTimeType > ettInvalid && < ettTotal") ----

// EXTERIOR_TIME_TYPE -- order pinned by nPrice[] memory layout in
// LoadExteriorInfoTable (Permanent group first) AND by Add() semantics
// (0=permanent, 1=+7days, 2=limited).
enum EXTERIOR_TIME_TYPE
{
    ettInvalid   = -1,
    ettPermanent = 0,   // Add: expire=0, flag=0
    ett7Days     = 1,   // Add: expire += 604800s
    ettLimit     = 2,   // Add: expire = max(existing, param)
    ettTotal     = 3,
};

// EXTERIOR_PAY_CURRENCY_TYPE -- epctMoney pinned as array index 1 (the
// column validated ">0" and multiplied x10000 in LoadExteriorInfoTable).
// epctFree / epctCoin names are INFERRED (values pinned); finalized when
// the buy chain is decompiled in slice #8.
enum EXTERIOR_PAY_CURRENCY_TYPE
{
    epctInvalid = -1,
    epctFree    = 0,    // name inferred: the "*Free" columns
    epctMoney   = 1,    // PINNED: "*0" columns, validated + x10000
    epctCoin    = 2,    // name inferred: "*1" columns, payType==2 path
    epctTotal   = 3,
};

// EXTERIOR_BUY_SOURCE -- pinned from `nBuySource > ebsInvalid && < ebsTotal`
// (valid 1..3) in the buy chain. Member names inferred; values pinned.
enum EXTERIOR_BUY_SOURCE
{
    ebsInvalid   = 0,
    ebsBuyDirect = 1,   // in-game money / bound currency
    ebsBuyBind   = 2,
    ebsBuyShop   = 3,   // coin shop (Exteriorshop.tab)
    ebsTotal     = 4,
};

// EXTERIOR_BUY_RESPOND_CODE -- observed return constants from the buy chain.
enum EXTERIOR_BUY_RESPOND_CODE
{
    ebrcOK              = 1,
    ebrcFailed          = 2,
    ebrcForceMismatch   = 5,
    ebrcFree            = 8,
    ebrcCoinPending     = 12,
    ebrcSystemDisabled  = 18,
    ebrcStateBlocked    = 19,
};

// ---- Capacity constants (pinned) ----
#define MAX_EXTERIOR_LIST_SIZE      0x2000  // 8192, DB save cap (SaveExteriorBox)
#define MAX_EXTERIOR_ITEM_COUNT     0x1fff  // 8191, in-memory box cap (Add)
#define EXTERIOR_7DAYS_SECONDS      0x93a80 // 604800s = 7 days (ett7Days step)
#define MAX_EXTERIOR_FREE_COUNT     60000   // free-use counter cap (AddExteriorFreeCount)
#define MAX_EXTERIOR_LATEST_BUY     20      // recent-buy list display cap

// ---- In-memory box item -- stride 0xc (12B), pinned via _Add + sync loop ----
struct KEXTERIOR_ITEM
{
    DWORD   dwID;           // +0x00  exterior ID
    DWORD   nFlag;          // +0x04  permanent/limited flag
    time_t  nExpireTime;    // +0x08  expire (0 = permanent). 32-bit build: 4B
};                          // sizeof = 0x0c

// ---- Config-table row (ExteriorInfo.tab), stored in the KExterior map
//      keyed by ID (ID is the map KEY, NOT a struct member).
//      18 dwords = 0x48, pinned from LoadExteriorInfoTable (the 0x12-dword
//      copy) + ApplyExteriorRepresent reads (+0xc/+0x10/+0x14). ----
struct KEXTERIOR_INFO
{
    DWORD   nForceID;       // +0x00
    DWORD   nGenre;         // +0x04
    DWORD   nSet;           // +0x08
    DWORD   nSubType;       // +0x0c  validated against equip subtype on apply
    DWORD   nRepresentID;   // +0x10  written into represent slot
    DWORD   nColorID;       // +0x14  written into color slot
    // nPrice[time][pay]: memory order Permanent(+0x18) / 7Days(+0x24) / Limit(+0x30);
    // within each: [epctFree, epctMoney, epctCoin]. nPrice[i][epctMoney] x10000.
    int     nPrice[ettTotal][epctTotal];    // +0x18 .. +0x3b (9 ints)
    DWORD   nLimitType;     // +0x3c
    DWORD   nIconID;        // +0x40
    DWORD   nRepresentID1;  // +0x44
};                          // sizeof = 0x48

//////////////////////////////////////////////////////////////////////////
// DB serialization structs -- byte layout must match v246 exactly.
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)

// Box block: [WORD count][item x count]. sizeof(KEXTERIOR_DB_DATA)==2 is
// the header size asserted by LoadExteriorBox. The 16B item is the
// binary's nested type KEXTERIOR_DB_DATA::KEXTERIOR_INFO.
struct KEXTERIOR_DB_DATA
{
    struct KEXTERIOR_INFO           // 16B DB item (NOT the config row above)
    {
        WORD    wID;                // +0x00
        BYTE    byFlag;             // +0x02
        DWORD   dwExpireTime;       // +0x03  (unaligned; packed)
        BYTE    byReserved[9];      // +0x07 -> pad to 0x10
    };                              // sizeof = 0x10
    WORD            wCount;         // +0x00
    KEXTERIOR_INFO  items[0];       // +0x02
};                                  // header sizeof = 2

// Latest-buy block: [WORD count][item x count]; item = 8B {WORD id; pad[6]}.
struct KEXTERIOR_BUY_LATEST_DB_DATA
{
    struct KEXTERIOR_INFO           // 8B latest-buy item
    {
        WORD    wID;                // +0x00
        BYTE    byReserved[6];      // +0x02 -> pad to 0x08
    };                              // sizeof = 0x08
    WORD            wCount;         // +0x00
    KEXTERIOR_INFO  items[0];       // +0x02
};                                  // header sizeof = 2

// Set block: [WORD count][item x count]; item = 18B {5 WORD ids; 8B pad}.
// Pinned from SaveExteriorSet/LoadExteriorSet (FUN_08200ec4 / FUN_0820224a):
// per-set stride 0x12, 5 style ids then memset(8). After the items the block
// carries a trailing [BYTE currentSetID][WORD freeCounter].
struct KEXTERIOR_SET_DB_DATA
{
    struct KEXTERIOR_SET_INFO
    {
        WORD    wExteriorID[5];      // +0x00
        BYTE    byReserved[8];       // +0x0a -> pad to 0x12
    };                               // sizeof = 0x12
    WORD                wCount;
    KEXTERIOR_SET_INFO  items[0];
};                                   // header sizeof = 2

// Client-sync wire item for the box-data packet (7B), pinned from
// DoSyncExteriorBoxData (FUN_08059bf0): {WORD id; BYTE flag; DWORD expire}.
struct KEXTERIOR_SYNC_ITEM
{
    WORD    wID;                // +0x00
    BYTE    byFlag;             // +0x02
    DWORD   dwExpireTime;       // +0x03 (unaligned) -> 7 bytes
};

#pragma pack()
// EXTERIOR_BUY_SOURCE / EXTERIOR_BUY_RESPOND_CODE enums deferred to slice #8
// (buy chain), where their members are pinned from the buy functions.

#endif  // _KEXTERIOR_DEF_H_
