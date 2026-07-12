# KRankListServer — INTEGRATION (ARCHITECTURE.md §8 filled in)

NEW subsystem, Server-only (`#ifdef _SERVER` sites). Singleton on KSO3World (not per-player, no DB).

## A. Binding points
| §8 | point | status | file:line | note |
|----|-------|--------|-----------|------|
| A1 | config .tab | N/A | — | no static config; data comes from the network feed |
| B2/B3 | KPlayer embed | N/A | — | not per-player |
| — | singleton member | [DONE] | KSO3World.h (KRankListServer m_RankListServer, beside m_StatDataServer) | include `../../Source/Common/SO3World/Src/KRankListServer.h` |
| — | Init | [DONE] | KSO3World.cpp KSO3World::Init (after m_StatDataServer.Init, infallible+last) | |
| — | UnInit | [DONE] | KSO3World.cpp KSO3World::UnInit (before m_StatDataServer.UnInit) | |
| C4-C7 | role-block DB | N/A | — | in-memory only, rebuilt from feed |
| D8/D10 | Lua shape | [DONE] | KLuaPlayer.cpp — shape (c) KPlayer method; REGISTER_LUA_FUNC(KPlayer, GetSingleDungeonScoreRankPos) | reads m_dwForceID + m_dwID |
| E12/E13 | packet | [DEFER] | feed handlers FUN_080d9514/080d94f4 + register FUN_080e5f50 (internal proto) | center-push, cross-process |
| G15 | apply hook | N/A | — | query-only |
| H17 | version-gap | note | | feed needs the SO3GameCenter rank-push port |

## B. Callee (what ported fns call OUT to)
| fn | calls | note |
|---|---|---|
| Init/UnInit | memset | infallible |
| SyncPlayerInfo/Finish/GetPos | none (self-contained array ops) | no cross-subsystem dependency |
| Lua wrapper | g_pSO3World->m_RankListServer.GetSingleDungeonRankListPos + m_dwForceID/m_dwID | KCharacter base fields |

## C. Caller (who calls IN)
- Lua content: `player:GetSingleDungeonScoreRankPos()` -> own rank in own force (0 = not ranked).
- KSO3World::Init/UnInit -> Init/UnInit.
- **DEFERRED:** center internal-protocol -> SyncPlayerInfo (per entry) + SyncFinish (batch end).

## D. Drift to LOCK
| assumption | value | guard |
|---|---|---|
| KSingle_Dungeon_Score_Player_Info size | 0x30 (48) | oracle `_CHK[sizeof==0x30]` |
| class size | 0x5de8 (24040) | oracle `_CHK[sizeof(KRankListServer)==0x5de8]` |
| PLAYER_FORCE_MAX_COUNT | 10 | binary assert string |
| SCORE_RANK_MAX_COUNT | 50 (0x32) | binary assert string |
| force 9 (丐帮) unplayable | MainKungfuInfo.tab has no kungfu for force 9 | data-fact; slot[8] always empty |

## E. Defer / data-gap
- **Feed protocol (cross-process):** the 2 GS receive-handlers + their internal-proto registration +
  the SO3GameCenter-side rank computation/push. Deferred with SO3GameCenter port. Leaderboard empty
  until then; `GetRankListPos` returns 0 (valid). See WORKLOG [RE-5]/DEFER.
