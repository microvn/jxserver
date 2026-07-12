# KRankListServer — PORT_DESIGN

Small NEW leaf; full RE + logic in WORKLOG.md. This file = the dependency/order/verify summary.

## 1. Dependency graph (bé→to)
```
KSingle_Dungeon_Score_Player_Info (48B struct)   <-- leaf
      |
KRankListServer class (Init/UnInit/Sync*/GetPos, [10][50] array + count[10])
      |
      +-- singleton member on KSO3World  (Init/UnInit chain)
      +-- Lua query binding (KPlayer method GetSingleDungeonScoreRankPos)
      +-- [DEFER] feed: center push -> 2 GS internal-proto handlers -> Sync*
```

## 2. Data structures (pinned from DWARF)
- `KSingle_Dungeon_Score_Player_Info` 0x30: dwPlayerID@0, szPlayerName[32]@4, dwMaxLevel@0x24,
  dwTotalScore@0x28, nEquipScore@0x2c.
- `KRankListServer` 0x5de8: DWORD count[10]@0, entry info[10][50]@0x28.
- Offsets are FREE (in-memory, no serialize) — the struct copy in SyncPlayerInfo is layout-agnostic;
  only sizes locked by oracle for parity with the DWARF ground-truth.

## 3. Persistence + packet
- **Persistence:** NONE (in-memory, rebuilt from feed). No role-block.
- **Packet:** feed = internal protocol {force, rank, 48B entry} + a finish signal. DEFERRED (center push).

## 4. Port order (single build; each verified)
1. Class + struct — oracle-mirror (layout + logic). DONE.
2. Singleton wire (KSO3World Init/UnInit) — build + boot. DONE.
3. Lua query binding + register — build + boot (no nil). DONE.
4. Coverage-diff — forgotten=0. DONE.

## 5. Remaining RE / Defer
- Feed protocol id + packet struct + center-side push = cross-process, defer with SO3GameCenter port.
- Exact v2.5 Lua binding name (best-effort `GetSingleDungeonScoreRankPos`).
