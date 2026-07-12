# KRegression — PORT_DESIGN

Returning-player (回归) reward subsystem. NEW in v2.5. Two classes:
`KRegressionManager` (singleton config, KSO3World member) + `KRegressionPlayerData` (per-player state).
Status: **RE complete, not yet ported.** Layout/logic from DWARF + decompile (see WORKLOG).

## §1 LINKED (dependency matrix)
```
KRegressionManager (config)  --Init<--  KSO3World::Init
   ^  (helpers: IsWork/GetRewardGradeID/GetRewardItemInfo/GetRegressionFinishedTime/IsCrossDays)
   |
KRegressionPlayerData (per-player)  --embed/Init<--  KPlayer
   |  Calculate  <--  KPlayer::OnExtDataLoadFinish (login post-load)
   |  Save/LoadRegression  <-->  KPlayer::Save/Load role-block rbtRegressionData(33)
   |  AddRewardItem --> CallAddRewardItemScript --> Lua PlayerScript.lua:AddRegressionRewardItem
   |  Calculate/AddRewardItem --> KPlayerServer::DoSyncRegressionPlayerData (DEFER: client 2.5-only)
   |
Lua bindings (KPlayer methods, shape c): GradeID/DailyCount/Data/Finished/AddReward
```
Leaf-first order: config structs -> Manager helpers -> Manager Load/Init -> PlayerData fields ->
Save/Load -> Calculate -> AddRewardItem/script -> Lua -> (defer packet).

## §2 Data structures (pinned from DWARF — see WORKLOG [RE-1])
- `KRegressionManager` 0x34: workflag, start/end, dailyCycle/Offset, resetCycle/Offset, m_RewardMap.
- `KRegressionPlayerData` 0x28: account{gradeID,ver,end}, playerAccountVer(transient),
  newPlayerEnd, currentGradeID, dailyCount, byItemMark[8], m_pPlayer.
- Config maps (from RegressionReward.tab, not serialized):
  `map<GradeID, {nLossDaily, map<KungFu, {bFreeLimit, nRegressionDailyCount,
   map<idx, KRewardItem{dwItemType[8],dwItemIndex[8],nItemStackNum[8]}>}>}>`.
- Constants: `REGRESSION_DAILY_MAX_COUNT=8`, `REGRESSION_ITEM_MAX_COUNT=8`.

## §3 Persistence + packet
Role block `rbtRegressionData` = 33 (append before rbtTotal). 2010 has no chunk key ->
**combine both v246 chunks into one block, account-first**:
```
KACCOUNT_REGRESSION_INFO  22B: [byGradeID][DWORD nEndTime][byVer][reserved16]   (#pragma pack(1))
KREGRESSION_DB_DATA       46B: [DWORD nNewPlayerEnd][byDaily][ItemMark8][byGrade][reserved32]
                               total block = 68 bytes
```
SaveRegression: write account 22B then player 46B (cap daily to 0xFF, grade/ver fit BYTE).
LoadRegression: read account 22B then player 46B; tolerate trailing reserved.
(If matching v246 chunk framing is later required, split into 2 chunk-keyed blocks — not needed for 2010.)
Packet DoSyncRegressionPlayerData(id, gradeID, dailyCount, mark[8]) = **DEFER** (stub no-op).

## §4 Port order (bé -> to), each slice + verify
1. **Config structs + constants** (KRegressionManager.h/KRegressionPlayerData.h).
   Verify: oracle-mirror static_assert sizeof (0x34,0x28,0x60,0x20,0x1c) + block sizes 22/46.
2. **Manager helpers** IsWork/IsCrossDays/GetRegressionFinishedTime/GetRewardGradeID/GetRewardInfo/
   GetRewardItemInfo. Verify: oracle-mirror the arithmetic (dayAlign, grade scan) vs WORKLOG [RE-5].
3. **Manager LoadConstList/LoadRegressionReward/CheckRegressionReward/Init**.
   Verify: build + boot -> still "Load game settings ... [OK]" (Init in KSO3World::Init chain; needs
   settings/RegressionReward.tab present, else Init fails -> guard/ship a minimal .tab).
4. **PlayerData fields + Init + Get* + GetItemMark + Save/LoadRegression**.
   Verify: oracle-mirror Save->Load roundtrip byte-exact (68B block).
5. **Calculate** + hook in KPlayer login path (reuse m_nLastLoginTime/m_nCurrentLoginTime).
   Verify: build + boot; oracle-mirror the branch table vs [RE-7].
6. **AddRewardItem + CallAddRewardItemScript + Lua bindings + REGISTER_LUA_FUNC**.
   Verify: build + boot (no nil / no metatable crash) + test_regression.lua calling
   GetPlayer():GetRegressionGradeID()/GetRegressionData().
7. **Coverage-diff** close-out (below).

## §5 Remaining RE
- Confirm the exact 2010 login post-load hook that runs after all role blocks load (v246
  OnExtDataLoadFinish). Candidates: `KPlayer::CallLoginScript()` (KPlayer.cpp:2076) or the
  LoadExtRoleData completion path. Must run once, post-load, pre-enter-scene.
- KRewardItemInfo inner `ItemInfoMap` key semantics (idx vs 0) — only matters if content uses
  multiple item-rows per (grade,kungfu); the 8-slot arrays inside one KRewardItem cover the
  observed case. Confirm from a real RegressionReward.tab if multi-row per kungfu appears.
- player+0xcc8 = "free-limit bypass" flag (VIP/paid) — map to the 2010 KPlayer equivalent.
- Calculate arg2==0 (account) vs !=0 (player) dispatch: confirm the account recompute is invoked
  separately (some caller passing 0) or only the player path is hit at login.

## §6 Defer
- Client sync packet + OnAddRegressionRewardItem (client 2.5-only). Stub.
- True account-level cross-character sharing (needs center account store). Per-character copy for now.
- Buy/grant economics: none — item grant is entirely in Lua PlayerScript.lua.

## §7 Coverage target (run at close-out)
Implemented set A = `grep -rhoE 'KRegression(Manager|PlayerData)::[A-Za-z_]+' src/.../KRegression*.cpp`.
Full surface B = nm on SO3GameServerD (see WORKLOG [RE-2], ~23 methods across the 2 classes).
`comm -13 A B` must be empty except documented defers (packet sync helpers).
Every non-implemented name = a deliberate, documented deferral. Report ported/folded/deferred/forgotten=0.
