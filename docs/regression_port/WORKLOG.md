# KRegressionManager + KRegressionPlayerData — RE WORKLOG

回归 (huí guī / hồi quy) = "returning-player" reward system. A player who has been away
(logged out) for N+ days gets classed into a *regression grade* and can claim a daily set of
reward items for a limited window. NEW subsystem in v2.5 (not in 2010 leak).

Source of layout = `/Volumes/ExData/game/jx3/jx3_dwarf/SO3GameServerD` (full DWARF, elf32-i386).
Source of logic = pyghidra `/SO3GameServer-3c8199` (stripped release, .text addresses match DWARF 1:1).
All offsets below carry DWARF evidence. Read-only RE — no source touched.

---

## [RE-0] Classification — NEW subsystem
`LC_ALL=C grep -rIna 'KRegressionManager::|KRegressionPlayerData' src include` = 0 hits. NEW.
Role-block enum in 2010 `include/Include/KRoleDBDataDef.h` ends at `rbtHairBoxData`; v246 has
`rbtRegressionData = 33`. Confirmed new both in code and persistence.

---

## [RE-1] Class layouts (llvm-dwarfdump --name=<T> -c)

### KRegressionManager — byte_size 0x34, decl KRegressionManager.h:53 (Source/Common/SO3World/Src)
Singleton, embedded by value in KSO3World (see [RE-6]).

| off  | field           | type        | note |
|------|-----------------|-------------|------|
| 0x00 | m_bWorkFlag     | BOOL (4)    | feature enabled + inside [start,end] window |
| 0x04 | m_nStartTime    | time_t (4)  | activity start (mktime of RegressionStartTime) |
| 0x08 | m_nEndTime      | time_t (4)  | activity end |
| 0x0c | m_nDailyCycle   | time_t (4)  | daily-reset period, default 86400 |
| 0x10 | m_nDailyOffset  | time_t (4)  | daily-reset anchor (+tzbias +345600) |
| 0x14 | m_nResetCycle   | time_t (4)  | grade/claim reset period, default 2592000 (30d) |
| 0x18 | m_nResetOffset  | time_t (4)  | reset anchor (+tzbias +345600) |
| 0x1c | m_RewardMap     | KREWARD_MAP | std::map<int GradeID, KRewardInfo> |

### KRegressionPlayerData — byte_size 0x28, decl KRegressionPlayerData.h:9
Per-player. Embedded in KPlayer at v246 offset +0x94e4 (offset is compiler-free; do not match).

| off  | field                         | type      | persisted in |
|------|-------------------------------|-----------|--------------|
| 0x00 | m_nAccountRegressionGradeID   | int       | account chunk (as BYTE) |
| 0x04 | m_nAccountRegressionVer       | int       | account chunk (as BYTE) |
| 0x08 | m_nPlayerAccountRegressionVer | int       | **NOT serialized** (transient) |
| 0x0c | m_nAccountRegressionEndTime   | time_t    | account chunk (DWORD) |
| 0x10 | m_nNewPlayerRegressionEndTime | time_t    | player chunk (DWORD) |
| 0x14 | m_nCurrentGradeID             | int       | player chunk (as BYTE) |
| 0x18 | m_nRegressionDailyCount       | int       | player chunk (as BYTE, capped 0xFF) |
| 0x1c | m_byItemMark[8]               | BYTE[8]   | player chunk (8B) — bitmask of claimed items per daily |
| 0x24 | m_pPlayer                     | KPlayer*  | back-ptr, not serialized |

### Config value structs (KRegressionManager.h) — loaded from .tab, NOT serialized
- **KRewardItem** (0x60): `DWORD dwItemType[8]@0` `int dwItemIndex[8]@0x20` `int nItemStackNum[8]@0x40`.
- **KRewardItemInfo** (0x20): `BOOL bFreeLimit@0` `int nRegressionDailyCount@4`
  `KREWARD_ITEM_INFO_MAP ItemInfoMap@8` (= std::map<DWORD, KRewardItem>).
- **KRewardInfo** (0x1c): `time_t nLossDaily@0` `KREWARD_ITEM_MAP ItemMap@4`
  (= std::map<int KungFu, KRewardItemInfo>).
- Nesting: `m_RewardMap[GradeID] -> {nLossDaily, ItemMap[KungFu] -> {bFreeLimit, nRegressionDailyCount,
  ItemInfoMap[idx] -> KRewardItem{8×(type,index,stack)}}}`.

Constants seen in asserts: `REGRESSION_DAILY_MAX_COUNT = 8`, `REGRESSION_ITEM_MAX_COUNT = 8`.

---

## [RE-2] Symbols (nm + c++filt -n)
Manager @0x0820xxxx: Init 0820b38e, UnInit 08209c30, IsWork(inlined@08209996),
IsCrossDays 08209c3e, GetRegressionFinishedTime 08209c82, GetRewardGradeID 08209e38,
GetRewardInfo 08209ce2, GetRewardItemInfo 08209f00, LoadConstList 0820af0c,
LoadRegressionReward 0820a2ea, CheckRegressionReward 08209d66.
PlayerData: Init 08208bb8, UnInit 08208bcc, GetGradeID 0833c922, GetDailyCount 0833c92e,
GetItemMark 08208bdc, SaveAccountData 08209106, LoadAccountData 08208d8c,
SavePlayerData 08209898, LoadPlayerData 082097de, Calculate 082092bc,
AddRewardItem 08208e58, CallAddRewardItemScript 08208c06.
Lua (KPlayer methods): LuaGetRegressionData 08316be0, LuaRegressionFinished 08316a06,
LuaAddRegressionReward 0831737e, LuaGetRegressionGradeID 0830e92c, LuaGetRegressionDailyCount 0830e8fa.
Packet (KPlayerServer): DoSyncRegressionPlayerData 0805e808, OnAddRegressionRewardItem 080619a0.

---

## [RE-3] Persistence — byte layouts (decompile SavePlayerData/LoadPlayerData/Save/LoadAccountData)

### KACCOUNT_REGRESSION_INFO — 22 bytes (0x16). SaveAccountData@08209106 / LoadAccountData@08208d8c
```
[0]     BYTE  byAccountRegressionGradeID   <- m_nAccountRegressionGradeID (low byte)
[1..4]  DWORD nAccountRegressionEndTime    <- m_nAccountRegressionEndTime  (time_t, full 4B)
[5]     BYTE  byAccountRegressionVer       <- m_nAccountRegressionVer (low byte)
[6..21] BYTE  reserved[16]                 memset 0
```
Load asserts leftSize >= 0x16 AND remaining == 0 after read (exact-size block).

### KREGRESSION_DB_DATA — 46 bytes (0x2e). SavePlayerData@08209898 / LoadPlayerData@082097de
```
[0..3]   DWORD nNewPlayerRegressionEndTime <- m_nNewPlayerRegressionEndTime (time_t, full 4B)
[4]      BYTE  byRegressionDailyCount       <- m_nRegressionDailyCount (capped to 0xFF on save)
[5..12]  BYTE  ItemMark[8]                  <- m_byItemMark[8]  (memcpy 8)
[13]     BYTE  byCurrentGradeID             <- m_nCurrentGradeID (low byte)
[14..45] BYTE  reserved[32]                 memset 0
```
Load asserts leftSize >= 0x2e. Save caps DailyCount to 255 first (`if(0xff < daily) daily=0xff`).

NOTE: m_nPlayerAccountRegressionVer (0x8) written by NEITHER block — transient, reset each session.
Grade/ver/daily are BYTE-truncated on disk ⇒ their runtime values must stay < 256.

---

## [RE-4] Config loading

### LoadConstList@0820af0c — INI, section [REGRESSION]
Opens `settings/<KSO3World+0x68780>` (the versioned const-list ini, "ServerConstListByVer.ini"
per KSO3World::Init strings). Keys (default in parens):
- `RegressionWorkFlag`  -> m_bWorkFlag
- `RegressionDailyCycle` (86400)  -> m_nDailyCycle  (assert > 0)
- `RegressionDailyOffset` (25200) -> m_nDailyOffset; then += tzBias(DAT_084df688) + 345600
- `RegressionResetCycle` (2592000)-> m_nResetCycle  (assert > 0)
- `RegressionResetOffset` (25200) -> m_nResetOffset; then += tzBias + 345600
- `RegressionStartTime`  -> 6-int date array {Y,M,D,h,m,s} -> mktime -> m_nStartTime
- `RegressionEndTime`    -> mktime -> m_nEndTime
- final: if workflag && (now < start || now > end) -> m_bWorkFlag = 0.

### LoadRegressionReward@0820a2ea — TABLE `settings/RegressionReward.tab`
Row per record from row index 2 (row 1 = header). Columns:
- `GradeID` (int, must != -1) — key of m_RewardMap
- `LossDaily` (int >= 0)      — days-away threshold -> KRewardInfo.nLossDaily
- `RegressionDaily` (int > 0) -> KRewardItemInfo.nRegressionDailyCount
- `FreeLimit` (int -> BOOL)   -> KRewardItemInfo.bFreeLimit
- `KungFu` (int)              — key of inner ItemMap (a grade can have per-kungfu rows)
- `ItemType%d`,`ItemIndex%d`,`ItemStackNum%d` for %d=1..8 -> KRewardItem arrays[8]
Consistency checks in loader: same GradeID across kungfu rows must agree on nLossDaily;
(GradeID==0) XOR (LossDaily==0) is an error ("New Player GradeID" error).

### CheckRegressionReward@08209d66 — validation after load
Iterates m_RewardMap ascending: nLossDaily must be strictly increasing between grades
(break/return 0 otherwise), and each grade's ItemMap size must be <= 8. Returns 1 on OK.

### Init@0820b38e
`LoadConstList() && LoadRegressionReward() && CheckRegressionReward()` — all-or-nothing, BOOL.

---

## [RE-5] Manager helpers (decompiled)
- **IsWork** (inlined @08209996): return m_bWorkFlag.
- **IsCrossDays(t1,t2)@08209c3e**: `dayAlign(t1, m_nDailyOffset, m_nDailyCycle) + m_nDailyCycle <= t2`
  i.e. t2 falls in a later daily window than t1. (dayAlign = t - (t - offset) % cycle)
- **GetRegressionFinishedTime(now)@08209c82**:
  `m_nResetCycle + (now - (now - m_nResetOffset) % 86400) + 86400`.
- **GetRewardGradeID(nLossDays)@08209e38**: if nLossDays==0 -> 0; else scan m_RewardMap ascending,
  keep GradeID whose nLossDaily != 0 && nLossDaily <= nLossDays (highest match wins); else -1.
- **GetRewardInfo(gradeID)@08209ce2**: m_RewardMap.find(gradeID) -> KRewardInfo*.
- **GetRewardItemInfo(gradeID, kungfu)@08209f00**: GetRewardInfo then ItemMap.find(kungfu).

---

## [RE-6] Manager singleton wiring — KSO3World member
DAT_084f67f8 = g_pSO3World. All accessors use `g_pSO3World + 0x61e30` as the manager `this`.
`KRegressionManager::Init()` is called inside **KSO3World::Init(IRecorderFactory*)** (FUN_0818f592,
confirmed by __assert `"Src/KSO3World.cpp" ... KSO3World::Init`) at 0x81902f1, as one of ~30
manager Init calls; on the failure-rollback path it calls `KRegressionManager::UnInit()`
(0x08209c30) guarded by its own init flag. => direct KSO3World member, KMentorCache pattern
(NOT a KWorldSettings config-table manager). Milestone still ends "Load game settings ... [OK]".

---

## [RE-7] Per-player hook — KPlayer::OnExtDataLoadFinish (login post-load)
Calculate is called from exactly one place: **KPlayer::OnExtDataLoadFinish()** (FUN_0839fb50,
confirmed by __assert string), after role blocks are loaded, before the player enters the scene:
```
Calculate(this+0x94e4, player+0xaf8 /*previous login/logout*/, player+0xad8 /*current login*/)
```
Player time fields: v246 +0xad8 = current login time, +0xaf8 = previous, +0xadc/+0xae0 also read
inside Calculate for the daily-cross check. 2010 equivalents: `m_nCurrentLoginTime` (KPlayer.h:258)
and `m_nLastLoginTime` (:257). Reuse those; do not transcribe raw offsets.

### Calculate@082092bc (state machine, run at login)
now = g_pSO3World+0x10 (world current time). If !IsWork() return.
- **Account path (arg2 == 0)**: recompute account grade = GetRewardGradeID(now - arg1);
  set finish deadline = GetRegressionFinishedTime(now); reset ItemMark; bump account ver on new grade.
- **Player path (arg2 != 0)**: if current account-end passed -> re-grade + IsCrossDays daily bump;
  else expire; sync player-account-ver vs account-ver (asserts ver>=playerVer), reset ItemMark on
  version bump. Always ends by calling **DoSyncRegressionPlayerData(playerId, gradeID, dailyCount,
  &ItemMark)** to push state to client.

---

## [RE-8] Reward claim + Lua

### AddRewardItem(nDailyIndex, nItemIndex)@08208e58
IsWork() gate; assert both indices in [0,8); if ItemMark[nDailyIndex] bit(nItemIndex) already set,
no-op. GetRewardItemInfo(currentGrade, nDailyIndex); check `nRegressionDailyCount <= m_nRegressionDailyCount`
and (bFreeLimit==0 || player+0xcc8 /*VIP/paid flag*/ == 0); read KRewardItem type/index/stack for that
kungfu slot; call CallAddRewardItemScript(type,index,count); on success set the mark bit and
DoSyncRegressionPlayerData.

### CallAddRewardItemScript(itemType,itemIndex,count)@08208c06
Calls Lua **`scripts/player/PlayerScript.lua` :: `AddRegressionRewardItem(player, type, index, count)`**.
Actual item granting lives in Lua; C++ only invokes + reads bool return. (Buy/grant = script-driven.)

### GetItemMark(nDailyIndex)@08208bdc: return m_byItemMark[idx] (idx in [0,8)), else 0.

### Lua bindings — KPlayer methods, shape (c) (all take `this = param_1 + 0x94e4`)
- `LuaGetRegressionGradeID`@0830e92c    -> GetGradeID() (m_nCurrentGradeID)
- `LuaGetRegressionDailyCount`@0830e8fa -> GetDailyCount() (m_nRegressionDailyCount)
- `LuaAddRegressionReward`@0831737e     -> args(nDailyIndex 1..8, nItemIndex 1..8, dwParam);
   asserts top==3, converts to 0-based, calls AddRewardItem.
- `LuaRegressionFinished`@08316a06      -> pushes bool: all reward marks for current grade claimed.
- `LuaGetRegressionData`@08316be0       -> builds nested Lua table for UI: per daily (up to 8):
   {nDailyCount, bFreeLimit, bCanHave, dwKungFuID, nItemCount=8,
    tItemTab={ per item: dwItemType, dwItemIndex, nItemCount, bCanHave, bUsed }, bAllUsed}.
   bCanHave gated by dailyCount and VIP flag (player+0xcc8); bUsed from ItemMark bits;
   dwKungFuID from player current kungfu (player+0x53e8).

## [PORT-1..4] DONE (core) — config + persist + getters
- KRegressionManager.h/.cpp: 4 config struct (KRewardItem 0x60 pinned), LoadConstList (ini
  [REGRESSION], StartTime 6-int→mktime), LoadRegressionReward (nested map grade→kungfu→item),
  CheckRegressionReward, helpers (IsCrossDays/GetRegressionFinishedTime/GetRewardGradeID/GetReward*).
- KRegressionPlayerData.h/.cpp: fields + Init + GetGradeID/GetDailyCount/GetItemMark + Save/Load
  (68B block = account 22B + player 46B, gộp 2 v246 chunk).
- WIRE: KSO3World member m_RegressionManager + Init in KSO3World::Init chain (after m_Settings);
  KRoleDBDataDef rbtRegressionData; KPlayer embed m_RegressionData + Init + role-block save/load;
  KLuaPlayer 2 getter (GetRegressionGradeID/DailyCount) + register.
- VERIFY: oracle layout (KRewardItem 0x60) + Save/Load 68B roundtrip PASS; build **ok=199**;
  boot **settings-OK** (after fixing a mis-read assert — see below).
- BUG caught by boot: CheckRegressionReward had `ItemMap.size()<=8` (mis-read from RE) but ItemMap
  is keyed by KUNGFU and real data has >8 kungfu/grade → dropped that check (8-cap is on daily items,
  array-bounded, not kungfu count). Boot net earned its keep.

## [S5/S6] DEFERRED (need RE-4 cross-check 2010) — documented, NOT forgotten
- Calculate (login re-grade) + its hook: needs the 2010 equivalent of KPlayer::OnExtDataLoadFinish
  (candidates KPlayer.cpp:2076 CallLoginScript / LoadExtRoleData completion) — unconfirmed.
- AddRewardItem + CallAddRewardItemScript (reward claim → PlayerScript.lua) + nested-UI
  LuaGetRegressionData: needs VIP flag (v246 player+0xcc8) + current-kungfu accessor 2010.
- DoSyncRegressionPlayerData packet (client 2.5-only). Stub.
- COVERAGE: impl 15 / binary 24 → 4 ctor/dtor implicit + 4 folded (Save/Load*Data→Save/Load) +
  3 deferred (Calculate/AddRewardItem/CallAddRewardItemScript). Forgotten=0.
