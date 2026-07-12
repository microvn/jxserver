# KDesignation (称号 / danh hiệu) — DRIFT port WORKLOG

RE-only pass (2026-07-12). Classification = **DRIFT** (class exists in both 2010 and v2.5).
Binary oracle: `/Volumes/ExData/game/jx3/jx3_dwarf/SO3GameServerD` (DWARF layout) +
`/SO3GameServer-3c8199` (stripped release, decompile by address — addresses match DWARF 1:1).

Summary of drift: v2.5 turns designations from a plain own/wear flag into a full
**timed + equip-gated + buff-granting** subsystem. New concepts:
- **Own vs Equip** split (Acquire = own it; Equip/UnEquip = wear it, one prefix + one postfix).
- **Timed designations** — a designation can expire (end-time maps, `Activate()` prunes expired).
- **Buff on equip** — equipping a prefix/postfix can add a KBuff + set a cooldown.
- **Independent (nType) prefix** — an "exclusive" prefix that forbids a postfix or byname display.

---

## [RE-1] Method surface diff (nm + c++filt)

2010 (`src/SO3World/Src/KDesignation.{h,cpp}`) = **15** members:
ctor, dtor, Init, UnInit, Save, Load, **SetCurrentDesignation(int,int,BOOL)**, AcquirePrefix(int),
AcquirePostfix(int), RemovePrefix(int), RemovePostfix(int), IsPrefixAcquired, IsPostfixAcquired,
SetGeneration(int), BroadcastDesignationAnnounce(int,int,BYTE).

v2.5 (DWARF) = **31** members. Kept-by-name (14): ctor, dtor, Init, UnInit, Save, Load,
AcquirePrefix (SIG DRIFT → `(int, time_t)`), AcquirePostfix(int), RemovePrefix, RemovePostfix,
IsPrefixAcquired, IsPostfixAcquired, SetGeneration, BroadcastDesignationAnnounce (SIG DRIFT → `int` type).
Removed in v2.5 (1): `SetCurrentDesignation` (split into SetCurrentPrefix/SetCurrentPostfix/SetBynameDisplayFlag).

**17 NEW methods (the "~16 missing"):**
| # | method | addr (release) | role |
|---|--------|------|------|
| 1 | `SetCurrentPrefix(int)` | 081cdc04 | set equipped prefix (+ add buff, + set independent flag) |
| 2 | `SetCurrentPostfix(int)` | 081cda30 | set equipped postfix (+ add buff) |
| 3 | `SetBynameDisplayFlag(int)` | 081cef60 | toggle byname display (blocked if independent) |
| 4 | `EquipPrefix(int)` | 081cf132 | CanEquip → SetCurrent → ResetCD → sync |
| 5 | `EquipPostfix(int)` | 081cf002 | symmetric |
| 6 | `UnEquipPrefix()` | 081cf5be | CD-gate → remove buff → clear → sync |
| 7 | `UnEquipPostfix()` | 081cf262 | symmetric |
| 8 | `CanEquipPrefix(int)` | 081cdee2 | gate: none equipped + acquired + (independent⇒no postfix/byname) |
| 9 | `CanEquipPostfix(int)` | 081cd878 | gate: none equipped + acquired |
| 10 | `GetPrefixEndTime(int,int*)` | 081cd7c2 | lookup m_PrefixEndTimeTable |
| 11 | `GetPostfixEndTime(int,int*)` | 081cd664 | lookup m_PostfixEndTimeTable |
| 12 | `ResetPrefixCDTime()` | 081ce17a | set player cooldown from pInfo->dwCoolDownID |
| 13 | `ResetPostfixCDTime()` | 081ce0a4 | symmetric |
| 14 | `Activate()` | 081cf920 | prune expired prefixes/postfixes (called per tick/login) |
| 15 | `LoadCurrentDesignation(KDesignationDB*)` | 081cddea | apply the 8-byte header (calls SetCurrentPrefix/Postfix) |
| 16 | `LoadEndTimeInfo(size_t*,BYTE*,size_t)` | 081ce386 | deserialize the NEW optional end-time block |
| 17 | `SaveEndTimeInfo(size_t*,BYTE*,size_t)` | 081ce64e | serialize the NEW optional end-time block |

---

## [RE-2] Class layout (DWARF, `KDesignation` @0x021812b7) — v2.5 size 0x90 (144B)

| offset | field | type | 2010? |
|--------|-------|------|-------|
| 0x00 | `m_bAllowBroadcastAnnounceFix` | BOOL | **NEW** (Init sets =1; gates announce in Acquire) |
| 0x04 | `m_AcquiredPrefix` | KCustomData<32> | same |
| 0x24 | `m_AcquiredPostfix` | KCustomData<32> | same |
| 0x44 | `m_pPlayer` | KPlayer* | same |
| 0x48 | `m_PrefixEndTimeTable` | `std::map<int,long>` (24B) | **NEW** id→time_t |
| 0x60 | `m_PostfixEndTimeTable` | `std::map<int,long>` (24B) | **NEW** |
| 0x78 | `m_bCurrentIndependent` | BOOL | **NEW** (cached pInfo->nType of equipped prefix) |
| 0x7c | `m_nCurrentPrefix` | int | same |
| 0x80 | `m_nCurrentPostfix` | int | same |
| 0x84 | `m_nGenerationIndex` | int | same |
| 0x88 | `m_nBynameIndex` | int | same |
| 0x8c | `m_bBynameDisplay` | BOOL | renamed from 2010 `m_bDisplayFlag` |

**Layout drift is FREE** (compiler-chosen offsets; recompile absorbs it). Only the serialized DB
blob + the config .tab column order are hard constraints (see [RE-4],[RE-5]).

## [RE-3] KDesignationDB header struct (DWARF @0x021812c3) — size 0x08, IDENTICAL to 2010
byCurrentPrefix@0 (BYTE), byCurrentPostfix@1 (BYTE), byGenerationIndex@2 (BYTE),
nBynameIndex@3 (int, unaligned, `#pragma pack(1)`), byBynameDisplay@7 (BYTE). 8 bytes.
2010 `KDesignationDB` (KDesignation.cpp:10) is byte-for-byte the same (only field name
byDisplayFlag→byBynameDisplay). **No header drift → no migration needed for the 8-byte prefix.**

---

## [RE-4] DB serialization (Save 081cfde8 / Load 081cfb50) — APPEND-ONLY, backward-compatible

**Save layout (v2.5):**
```
[ KDesignationDB : 8B ]              # header, identical to 2010
[ m_AcquiredPrefix  : 32B ]          # KCustomData<32>::Save
[ m_AcquiredPostfix : 32B ]          # KCustomData<32>::Save
[ EndTimeInfo block ]  <-- ONLY written if (m_PrefixEndTimeTable.size()!=0 || m_PostfixEndTimeTable.size()!=0)
```
So 72 bytes when no timed designations exist == exactly the old 2010 blob.

**Load layout:** read header (LoadCurrentDesignation) → 32B prefix bitmap → 32B postfix bitmap →
**if (uLeftSize != 0)** LoadEndTimeInfo, then require uLeftSize == 0.
⇒ Old 72-byte 2010 blobs load cleanly (leftover==0 → skip end-time). **Backward-compatible both ways.**
Note: 2010's Load ends with `KGLOG_PROCESS_ERROR(uLeftSize == DESIGNATIONFIX_DATA_SIZE)` style exact-checks;
the port must relax the final check to `if (uLeftSize != 0) LoadEndTimeInfo(...)` (v2.5 shape).

**EndTimeInfo block format (SaveEndTimeInfo 081ce64e / LoadEndTimeInfo 081ce386):**
```
[ BYTE nPrefixCount ]
[ { BYTE id ; time_t(4B) endtime } x nPrefixCount ]     # from m_PrefixEndTimeTable
[ BYTE nPostfixCount ]
[ { BYTE id ; time_t(4B) endtime } x nPostfixCount ]    # from m_PostfixEndTimeTable
```
Per-entry = 5 bytes (`sizeof(BYTE)+sizeof(time_t)`, time_t=4 on 32-bit). Both count bytes ALWAYS
present when the block is emitted (min block = 2 bytes). Load asserts each table empty before filling.
Map value is `long` (pair<int,long>, value@+4 in the node).

## [RE-5] Config drift — KPrefixInfo / KPostfixInfo + .tab columns

2010 struct = `{ BYTE byAnnounceType; }` only. v2.5 (DWARF):
- **KPrefixInfo** (0x18=24B): nAnnounceType@0 (int), dwCoolDownID@4, nOwnDuration@8, dwBuffID@0xc,
  nBuffLevel@0x10, **nType@0x14** (independent flag, prefix-only).
- **KPostfixInfo** (0x14=20B): same but NO nType.

LoadPrefixInfo (081d49f6) reads columns in order: `ID, AnnounceType(def 1), CoolDownID(def 0),
OwnDuration(def 0), BuffID(def 0), BuffLevel(def 0), Type`. Stores puVar1[0..5] = those 6 values
after ID. Postfix loader = same minus `Type`.

**Data present:** deploy tree `镜像端/extracted/root/settings/DesignationPrefixInfo.tab` header =
`ID  AnnounceType  CoolDownID  OwnDuration  BuffID  BuffLevel  Type`; postfix = same minus Type.
Sample rows only fill AnnounceType (buff/duration columns blank ⇒ 0) → timed/buff features are
data-driven and currently dormant in this leak's data, but the columns exist so the loader is safe.

---

## [RE-6] Behavior of the new methods (decompiled)

- **Init (081cd99e):** m_pPlayer=arg; zero 0x78..0x8c; **m_bAllowBroadcastAnnounceFix=1**. Maps default-empty.
- **SetCurrentPrefix(int) (081cdc04):** valid [0,255] (0=clear). If !=0: assert m_pPlayer;
  pInfo=GetPrefixInfo; if pInfo->dwBuffID(@0xc)!=0 → `m_pPlayer->m_BuffList(@0x9b0).AddBuff(dwID,mapID?,dwBuffID,nBuffLevel,...)`
  (FUN_0826bc20); if pInfo->nType(@0x14)!=0 → m_bCurrentIndependent(@0x78)=1; m_nCurrentPrefix=nPrefix.
- **SetCurrentPostfix(int) (081cda30):** symmetric, no nType branch.
- **CanEquipPrefix(int) (081cdee2):** require m_nCurrentPrefix==0, IsPrefixAcquired, pInfo ok;
  if pInfo->nType!=0 → require m_nCurrentPostfix==0 AND m_bBynameDisplay==false.
- **CanEquipPostfix(int) (081cd878):** require m_nCurrentPostfix==0 + IsPostfixAcquired (no nType branch).
- **EquipPrefix(int) (081cf132):** valid → CanEquipPrefix → SetCurrentPrefix → ResetPrefixCDTime →
  `g_PlayerServer.DoSyncPlayerDesignation(m_pPlayer, cur.prefix, postfix, gen, byname, display)`.
- **UnEquipPrefix() (081cf5be):** if equipped: pInfo=GetPrefixInfo; **CD gate** —
  `m_pPlayer->m_CoolDownList(@0x67f8).IsCoolDownReady?(pInfo->dwCoolDownID)` (FUN_0828034a); if not ready →
  `g_PlayerServer.SendNotify(playerObjID@0xa50, 0x1f/*=31*/, 3,...)` (FUN_08060e0c) + fail.
  Else: if dwBuffID → RemoveBuff (FUN_0826abd8); if nType → m_bCurrentIndependent=0; m_nCurrentPrefix=0; DoSync.
- **UnEquipPostfix() (081cf262):** symmetric, no nType.
- **SetBynameDisplayFlag(int) (081cef60):** if m_bCurrentIndependent==0 (else blocked): if changed → set + DoSync.
- **ResetPrefixCDTime() (081ce17a):** if equipped & pInfo->dwCoolDownID!=0 → look up recipe
  (g_pSO3World->m_?(DAT+0x2588), FUN_082a6194) → `m_pPlayer->m_CoolDownList.SetCoolDown(dwCoolDownID, ...)` (FUN_082803fc).
- **AcquirePrefix(int nPrefix, time_t nEndTime) (081cec1c):** valid [1,255]; pInfo=GetPrefixInfo.
  If nEndTime==0 (permanent path): if IsPrefixAcquired→return true; if pInfo->nOwnDuration!=0 →
  nEndTime = g_pSO3World->m_nCurrentTime(DAT+0x10) + nOwnDuration; m_PrefixEndTimeTable[nPrefix]=nEndTime.
  If nEndTime!=0 (explicit): assert nOwnDuration!=0 && nEndTime>now; cap nEndTime = min(nEndTime, now+nOwnDuration);
  if already present with same endtime→return true; m_PrefixEndTimeTable[nPrefix]=capped.
  Then: `m_AcquiredPrefix.SetBit(nPrefix,true)`; `g_PlayerServer.DoAcquireDesignation(m_pPlayer,nPrefix,0)`;
  **if m_bAllowBroadcastAnnounceFix** → BroadcastDesignationAnnounce(nPrefix,0,pInfo->nAnnounceType);
  `g_pSO3World->m_StatDataServer.UpdateDesignationStat(true,nPrefix)`.
- **AcquirePostfix(int) (081cea26):** 2010 keeps `AcquirePostfix(int)` — postfix has nOwnDuration too;
  mirrors prefix minus nType (verify: postfix end-time managed identically).
- **RemovePrefix(int) (081cf75c):** valid; if IsPrefixAcquired: if m_nCurrentPrefix==nPrefix →
  clear cooldown (m_CoolDownList.Clear(pInfo->dwCoolDownID), FUN_082802b2) + UnEquipPrefix();
  m_AcquiredPrefix.SetBit(false); DoRemoveDesignation. (Also erases end-time entry — see Activate erase path.)
- **RemovePostfix(int) (081cf3f6):** symmetric.
- **Activate() (081cf920):** iterate m_PrefixEndTimeTable: if endtime < g_pSO3World->m_nCurrentTime →
  RemovePrefix(id) + erase entry; same loop for m_PostfixEndTimeTable → RemovePostfix(id) + erase.
  = expire timed designations. Must be called each frame/login (find its 2010 caller site — likely
  `KPlayer::Activate`; NOT currently wired in 2010 since the method is new).
- **GetPrefixEndTime(int,int*) (081cd7c2):** if IsPrefixAcquired: *out=0; if in table → *out=table[id]; true.
- **BroadcastDesignationAnnounce(int,int,int byType) (081ce906):** byType==3(datGlobal) →
  g_RelayClient.DoDesignationGlobalAnnounceRequest(playerID, prefix, postfix); else
  g_PlayerServer.BroadcastDesignationAnnounce(playerID, m_pPlayer.m_szName(@0x58), prefix, postfix, byType).
  datGlobal==3 confirmed (Global.h:463 KDESIGNATION_ANNOUNCE_TYPE datInvalid=0,datNearby=1,datScene=2,datGlobal=3). No enum drift.
- **SetGeneration(int) (081ce9b8):** unchanged from 2010 (gen index + random byname + DoSetGenerationNotify).

## [RE-7] Lua binding drift (KLuaPlayer.cpp, kiểu (c) = KPlayer method)

v2.5 Lua funcs (21), m_Designation at KPlayer offset 0x9454 (release), e.g.
`LuaGetDesignationPrefixEndTime` (0831f7c6): nTop==1; nPrefix=arg1; GetPrefixEndTime→push or nil.

Kept (14): Acquire{Prefix,Postfix}, Get{Acquired}Designation{Prefix,Postfix,Count},
GetCurrentDesignation{Prefix,Postfix}, Is...Acquired{Prefix,Postfix}, Remove...{Prefix,Postfix},
GetDesignation{Byname,Generation}.
**NEW (7):** `LuaEquipDesignationPrefix/Postfix`, `LuaUnEquipDesignationPrefix/Postfix`,
`LuaGetDesignationPrefixEndTime/PostfixEndTime`, `LuaSetDesignationBynameDisplayFlag`.
**Renamed:** `LuaGetDesignationDisplayFlag`→`LuaGetDesignationBynameDisplayFlag`.
**Removed in v2.5 (3):** `LuaSetCurrentDesignation`, `LuaGetCurrentDesignation`, `LuaSetDesignationContent`.
