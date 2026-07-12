# KRankListServer (单人副本积分排行) — REPORT

**Status: DONE + verified** (class + singleton + Lua query). Feed protocol DEFERRED (cross-process).

## What it is
NEW v2.5 server singleton: an in-memory leaderboard of single-player-dungeon scores, indexed
`[10 forces][50 ranks]` of {playerID, name[32], maxLevel, totalScore, equipScore} (48B), plus a
per-force valid-count[10]. Fed by the center (rank push), queried by Lua (`player:GetSingleDungeonScoreRankPos()`).

## Metrics
- **Methods:** DWARF surface = 5. Implemented = 5 (+ ctor/dtor). **Forgotten = 0.**
- **Build:** ok=200 fail=0, link exit=0, undefined refs=0 (+1 object vs prior 199).
- **Boot:** `Load game settings ... [OK]` — new Init runs, no regression.
- **Files:** NEW KRankListServer.{h,cpp}; edited KSO3World.{h,cpp}, KPlayer.h, KLuaPlayer.cpp.

## Verify nets
- **Oracle-mirror:** layout (`sizeof entry==0x30`, `sizeof class==0x5de8`, offsets) + logic
  (Sync store, Finish trailing-trim incl. mid-gap, GetPos hit/miss/invalid-force, force-9-empty) ALL PASS.
- **Build host:** clean.
- **Boot no-regression:** settings-[OK].

## Risk
Low. No DB serialize (in-memory), no per-player state, no role-block, no buy chain. The only mutable
global is Init'd/UnInit'd cleanly in the KSO3World chain. Force 9 (丐帮) unplayable in this build ->
slot always empty (verified via MainKungfuInfo.tab, not a bug).

## Deferred (documented, cross-process)
- **Feed:** center-side rank computation + push, the 2 GS internal-protocol receive-handlers, and their
  registration. Belongs to the SO3GameCenter port. Leaderboard empty + GetRankListPos returns 0 until then.
- **Lua name:** exact v2.5 script name for the query binding not recoverable from the register table
  (macro-stringified, no plain name ptr) -> named `GetSingleDungeonScoreRankPos` by convention.

## Live-oracle follow-on (env-gated)
Feeding entries + querying rank end-to-end needs the center push (deferred) + a client. Oracle-mirror +
no-regression boot carry the slices; the query path is exercisable via a `test_ranklist.lua` once a
center feed exists.
