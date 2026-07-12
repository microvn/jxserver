# KRankListServer (单人副本积分排行 / xếp hạng điểm phó bản đơn) — port WORKLOG

Classification = **NEW** (`grep KRankListServer:: src` = 0). DWARF-based.
Binary oracle: `jx3_dwarf/SO3GameServerD` (layout+sig) + `SO3GameServer-3c8199` (decompile by addr).

## [RE-1] Surface (nm + c++filt) — 5 methods + ctor/dtor
| method | addr (release) | role |
|---|---|---|
| Init() | 082130a6 | memset count[](0x28) + info[][](24000) = 0 |
| UnInit() | 08213064 | same memset |
| SyncSingleDungeonScoreRankPlayerInfo(DWORD force, DWORD rank, KSingle_Dungeon_Score_Player_Info) | 08212f6e | store info[force-1][rank-1] (struct by value = 12 dwords) |
| SyncSingleDungeonScoreRankFinish() | 08212e24 | per force: count=50, trim trailing slots where dwMaxLevel==0 |
| GetSingleDungeonRankListPos(DWORD force, DWORD playerID) | 08212e9e | scan force's 50 slots for dwPlayerID -> 1-based rank; 0 if none |

## [RE-2] Class layout (DWARF) — byte_size 0x5de8 (24040)
| offset | field | type |
|---|---|---|
| 0x00 | m_dwSingleDungeonScoreRankCount[10] | DWORD[PLAYER_FORCE_MAX_COUNT] (per-force valid count) |
| 0x28 | m_SingleDungeonScoreRankPlayerInfo[10][50] | entry[FORCE][RANK] |

Entry `KSingle_Dungeon_Score_Player_Info` (byte_size 0x30=48): dwPlayerID@0, szPlayerName[32]@4,
dwMaxLevel@0x24, dwTotalScore@0x28, nEquipScore@0x2c. Check: 0x28 + 10*50*0x30 = 0x28+24000 = 0x5de8 ✓.

## [RE-3] Constants (from binary assert strings)
- `PLAYER_FORCE_MAX_COUNT` = 10 (dwForceID valid 1..10). NOT in 2010 source -> defined in header.
- `SCORE_RANK_MAX_COUNT` = 50 (0x32) (dwRank valid 1..50).
- The 10 forces = RelationForce.tab id 1..10 (少林/万花/天策/纯阳/七秀/五毒/唐门/藏剑/丐帮/明教).
  **DATA-FACT:** MainKungfuInfo.tab has kungfu for forces 1,2,3,4,5,6,7,8,10 only — **force 9 (丐帮/Cái
  Bang) has NO 心法 = unplayable in this v2.5.2 build** (reserved id, content shipped in a later
  expansion). So slot [8] (force 9) is always empty; not a bug.

## [RE-4] Logic (decompiled)
- **Init/UnInit:** memset both blocks to 0.
- **SyncPlayerInfo:** validate force[1,10] & rank[1,50]; `info[force-1][rank-1] = Info` (48-byte copy).
- **SyncFinish:** for force 9..0: count[force]=50; from rank 49 down, while slot.dwMaxLevel==0: count--.
  = trailing-empty trim (feeder pre-sorts top-down, empties cluster at the tail).
- **GetRankListPos:** validate playerID!=0 & force[1,10]; scan ranks 0..49 for matching dwPlayerID;
  return rank+1 (1-based) or 0.

## [RE-5] Wiring (xrefs + decompiled callers)
- **Singleton:** object at `g_pSO3World + 0x61e64` = KSO3World member. Init called inside KSO3World::Init
  (FUN_0818f592, the fn printing "Load game settings [OK]"); UnInit on the failure-rollback path.
- **Feeder (network):** FUN_080d9514 unpacks a packet {force@+2, rank@+6, 12-dword entry@+10..+0x36}
  -> SyncPlayerInfo; FUN_080d94f4 -> SyncFinish. Both registered via FUN_080e5f50/FUN_080e7a84
  (GS internal-protocol table). Data is PUSHED BY THE CENTER (cross-process). **DEFERRED** — see below.
- **Lua query:** FUN_08314218 = KPlayer method (shape c): reads this force (m_dwForceID) + id (m_dwID),
  validates force[1,10], calls GetRankListPos, pushes number. Zero Lua args (queries the caller's own rank).

## [PORT-1..3] Port (2026-07-13) — build ok=200 fail=0, boot settings-[OK]
- **[PORT-1] Class.** New `src/SO3World/Src/KRankListServer.{h,cpp}` — 5 methods + struct, exactly per
  RE. PLAYER_FORCE_MAX_COUNT=10 / SCORE_RANK_MAX_COUNT=50 defined in header. Oracle-mirror: layout
  (sizeof entry 48, class 0x5de8) + logic (Sync/Finish/GetPos incl. force-9-empty, mid-gap trim) ALL PASS.
- **[PORT-2] Singleton.** `KRankListServer m_RankListServer` in KSO3World.h (next to m_StatDataServer,
  include via canonical `../../Source/Common/SO3World/Src/KRankListServer.h`). `m_RankListServer.Init()`
  appended to KSO3World::Init chain (infallible+last -> no rollback flag); `UnInit()` in KSO3World::UnInit.
- **[PORT-3] Lua.** `KPlayer::LuaGetSingleDungeonScoreRankPos` (shape c) + REGISTER_LUA_FUNC, _SERVER.
  Reads m_dwForceID + m_dwID -> GetRankListPos. Exact v2.5 Lua NAME not recoverable (register table
  stores no plain name ptr; macro-stringified) -> named by convention `GetSingleDungeonScoreRankPos`.

## Coverage-diff — forgotten = 0
5/5 methods + ctor/dtor implemented. B-A empty.

## DEFER (network dependency, cross-process — documented, not forgotten)
The **feed** (SyncPlayerInfo/SyncFinish callers = 2 GS internal-protocol handlers + their registration +
the CENTER-side rank computation & push) is deferred: the pusher lives in SO3GameCenter (separate binary,
separate port), analogous to buy-chain-defers-on-currency. Until then the leaderboard stays empty and
`GetRankListPos` returns 0 (a valid "not ranked" answer). The query API + class + singleton are complete
and verified; only the data source is dormant.
