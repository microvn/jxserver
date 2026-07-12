# KDesignation DRIFT port — INTEGRATION (ARCHITECTURE.md §8 filled in)

This is a **DRIFT** port: KDesignation already exists in 2010 and is ALREADY fully wired
(embed, role-block, save/load, Lua register, sync packets). So most §8 binding points are
**already done** — the work is patching new methods + new fields into the existing hooks, NOT
adding a fresh subsystem. Below, each §8 point is marked `[EXISTS]` (already wired in 2010,
no new edit) or `[PATCH]` (needs a new edit for the drift).

## A. Binding points

| §8 | point | status | file:line (2010) | note |
|----|-------|--------|------------------|------|
| A1 | config Init (`KDesignationList::Init`) | [EXISTS] | KDesignationList.cpp:9; wired in `KWorldSettings` as `m_DesignationList` | LoadPrefixInfo/LoadPostfixInfo `[PATCH]` — read 5 new columns |
| B2 | KPlayer embed `m_Designation` | [EXISTS] | KPlayer.h:310 | no change (offset is free) |
| B3 | `m_Designation.Init(this)` | [EXISTS] | KPlayer.cpp:141 (+UnInit 455,577) | no change |
| C4 | role-block enum `rbtDesignationData` | [EXISTS] | KRoleDBDataDef.h:36 | **do NOT touch** — same tag, format is append-compatible |
| C5 | `SAVE_ROLE_BLOCK(m_Designation.Save,...)` | [EXISTS] | KPlayer.cpp:2363 | no change (Save signature unchanged) |
| C6 | `case rbtDesignationData:` load | [EXISTS] | KPlayer.cpp:2030-2031 | no change (Load signature unchanged) |
| C7 | DB byte layout | [PATCH] | KDesignation.cpp Save/Load | append optional EndTimeInfo block; relax Load tail check |
| D8 | Lua shape | [EXISTS] | kiểu (c) KPlayer method | keep shape; add 7 new + rename 1 |
| D10 | `REGISTER_LUA_FUNC(KPlayer, ...)` | [PATCH] | KLuaPlayer.cpp:11899-11922 | register 7 new, rename Get...DisplayFlag→...BynameDisplayFlag; DROP 3 removed (or keep for back-compat) |
| E12 | client sync packet (DoSyncPlayerDesignation etc.) | [EXISTS] | KPlayerServer.h:585-590 | 5 emit funcs already declared+used; no new packet needed |
| G15 | apply hook (buff/cooldown/representID) | [PATCH] | KDesignation.cpp SetCurrent*/UnEquip* | reuse `m_BuffList.AddBuff/RemoveBuff` + `m_CoolDownList` (KPlayer members) |
| — | `Activate()` tick hook | [PATCH-NEW] | KPlayer::Activate (find the per-player tick) | **NEW call site** — 2010 never called it; must add `m_Designation.Activate()` |

## B. Callee (what the new methods call OUT to → reuse 2010 mechanism)

| ported method | calls out to | 2010 mechanism to REUSE |
|---------------|--------------|-------------------------|
| SetCurrentPrefix/Postfix | add buff | `KPlayer::m_BuffList` AddBuff (already exists; args: buffID, level) |
| UnEquipPrefix/Postfix | remove buff | `KPlayer::m_BuffList` RemoveBuff |
| ResetPrefixCDTime, UnEquip (gate), RemovePrefix | player cooldown | `KPlayer::m_CoolDownList` Set/IsReady/Clear + recipe lookup via `g_pSO3World` cooldown-recipe mgr |
| Acquire*, Activate | current time | `g_pSO3World->m_nCurrentTime` |
| Acquire* | announce + stat | `g_PlayerServer.DoAcquireDesignation`, `BroadcastDesignationAnnounce`, `m_StatDataServer.UpdateDesignationStat` (all EXIST) |
| Equip*/UnEquip*/SetBynameDisplayFlag | client sync | `g_PlayerServer.DoSyncPlayerDesignation` (EXISTS, KPlayerServer.h:590) |
| BroadcastDesignationAnnounce | global announce | `g_RelayClient.DoDesignationGlobalAnnounceRequest` (used by 2010 already) |
| Config load | new columns | `KPrefixInfo`/`KPostfixInfo` gain 5 fields; ITabFile `GetInteger` per column |

## C. Caller (who calls IN)

- Lua content scripts: `player:EquipDesignationPrefix(id)`, `:AcquireDesignationPrefix(id[,endtime])`,
  `:GetDesignationPrefixEndTime(id)`, `:SetDesignationBynameDisplayFlag(flag)`, etc.
- `KPlayer::Save`/`Load` (role-block, already wired).
- `KPlayer::Activate` (per-player tick) → **must call `m_Designation.Activate()`** (new wiring).
- `KPlayer` display-data sync (KPlayer.cpp:1320-1324 copies m_nCurrent* — keep; note field
  `m_bDisplayFlag`→`m_bBynameDisplay` rename ripples here).

## D. Drift to LOCK (compile-time guards — `-std=gnu++98`, use `typedef char _CHK[COND?1:-1]`)

| assumption | value from binary | guard |
|------------|-------------------|-------|
| KDesignationDB size | 8 | `typedef char _CHK_DB[(sizeof(KDesignationDB)==8)?1:-1];` |
| KDesignationDB.nBynameIndex offset | 3 | `offsetof(KDesignationDB,nBynameIndex)==3` |
| DESIGNATIONFIX_DATA_SIZE | 32 (0x20) | already `MAX_DESIGNATION_FIX_ID/CHAR_BIT+1` = 32 ✓ |
| EndTime per-entry wire size | 5 (BYTE + time_t 4B) | assert in oracle-mirror roundtrip |
| KPrefixInfo size / nType offset | 24 / 0x14 | `_CHK[sizeof(KPrefixInfo)==24 && offsetof(...,nType)==0x14]` |
| KPostfixInfo size | 20 | `_CHK[sizeof(KPostfixInfo)==20]` |
| datGlobal ordinal | 3 | Global.h:463 confirmed (no drift) |
| time_t width | 4 (32-bit build) | model as DWORD in oracle-mirror |

## E. Defer / data-gap

- **Buff/cooldown/duration data is blank** in this leak's `.tab` (only AnnounceType filled). The
  buff-grant + timed-expiry paths are code-complete but dormant until data provides
  BuffID/CoolDownID/OwnDuration/Type. Not a code defer — a data-gap; note in REPORT.
- **`Activate()` tick frequency** — confirm the exact 2010 per-player tick that should call it
  (candidate: KPlayer::Activate). If unsure, calling on login + a low-frequency tick is safe.
- Removed Lua funcs (LuaSetCurrentDesignation, LuaGetCurrentDesignation, LuaSetDesignationContent):
  keep them for content back-compat OR drop to match v2.5 exactly — decide with the script tree.
