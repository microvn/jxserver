# KDesignation DRIFT — PORT_DESIGN

## 1. Dependency graph (bé→to)
```
KPrefixInfo/KPostfixInfo struct (config)        <-- leaf, port FIRST
      |
KDesignationList::LoadPrefixInfo/PostfixInfo    <-- read new columns
      |
KDesignation class fields (end-time maps, m_bAllowBroadcastAnnounceFix,
      m_bCurrentIndependent, rename m_bDisplayFlag->m_bBynameDisplay)
      |
      +-- SaveEndTimeInfo / LoadEndTimeInfo  --> Save/Load (DB blob)   [oracle roundtrip]
      +-- SetCurrentPrefix/Postfix (buff) --> EquipPrefix/Postfix
      |          CanEquipPrefix/Postfix -----^
      |          ResetPrefix/PostfixCDTime --^ (cooldown)
      +-- AcquirePrefix(int,time_t)/AcquirePostfix --> end-time maps + announce
      +-- UnEquipPrefix/Postfix (buff remove + CD gate)
      +-- Activate() (expire) --> RemovePrefix/Postfix
      +-- GetPrefix/PostfixEndTime
      |
Lua bindings (7 new + 1 rename) + REGISTER      <-- top, port LAST
Activate() call site in KPlayer::Activate        <-- top
```

## 2. Data structures (pinned from DWARF)

### KPrefixInfo (24B) / KPostfixInfo (20B) — `include/Include/SO3World/KDesignationList.h`
```c
struct KPrefixInfo {           // 0x18
    int   nAnnounceType;       // @0x00  (2010 was BYTE byAnnounceType; widen to int)
    DWORD dwCoolDownID;        // @0x04
    int   nOwnDuration;        // @0x08  seconds; 0 = permanent
    DWORD dwBuffID;            // @0x0c  0 = no buff
    int   nBuffLevel;          // @0x10
    int   nType;               // @0x14  !=0 => independent/exclusive prefix (prefix ONLY)
};
struct KPostfixInfo {          // 0x14 — identical minus nType
    int nAnnounceType; DWORD dwCoolDownID; int nOwnDuration; DWORD dwBuffID; int nBuffLevel;
};
```
.tab columns (order from LoadPrefixInfo 081d49f6): `ID AnnounceType CoolDownID OwnDuration BuffID BuffLevel Type`
(postfix: no Type). Deploy tree data already has these columns.

### KDesignation new members (offsets FREE; names/types matter)
```c
BOOL                 m_bAllowBroadcastAnnounceFix;   // Init=TRUE; gates Acquire's announce
std::map<int,time_t> m_PrefixEndTimeTable;           // id -> expiry
std::map<int,time_t> m_PostfixEndTimeTable;
BOOL                 m_bCurrentIndependent;          // cached nType of equipped prefix
// rename existing: m_bDisplayFlag -> m_bBynameDisplay
```

### KDesignationDB (8B, `#pragma pack(1)`) — UNCHANGED, keep 2010 struct
byCurrentPrefix, byCurrentPostfix, byGenerationIndex (BYTE x3), nBynameIndex (int), byBynameDisplay (BYTE).

## 3. Persistence + packet

**DB blob (role-block `rbtDesignationData`, unchanged tag):**
```
[DB 8B][AcquiredPrefix 32B][AcquiredPostfix 32B]  ( [EndTimeInfo] only if any map non-empty )
EndTimeInfo = [BYTE nPre]{BYTE id; time_t(4)}xnPre [BYTE nPost]{BYTE id; time_t(4)}xnPost
```
- Save: after the two bitmaps, `if (m_PrefixEndTimeTable.size() || m_PostfixEndTimeTable.size()) SaveEndTimeInfo(...)`.
- Load: after the two bitmaps, `if (uLeftSize != 0) { LoadEndTimeInfo(...); require uLeftSize==0; }`.
- **Backward-compatible:** old 72-byte blobs load fine (leftover 0 ⇒ end-time skipped). New blobs
  with no timed designations are byte-identical to old. NO migration, NO enum shift.

**Packets:** all client sync already exists (DoSyncPlayerDesignation / DoAcquireDesignation /
DoRemoveDesignation / DoSetGenerationNotify / DoSyncDesignationData, KPlayerServer.h:585-590).
No new protocol id needed. (S2C_DESIGNATION_ANNOUNCE already in GS_Client_Protocol.h:2451.)

## 4. Port order (each slice + verify)

1. **Config struct + loaders** — grow KPrefixInfo/KPostfixInfo, add 5-column reads.
   Verify: oracle-mirror `static_assert(sizeof==24/20, offsetof(nType)==0x14)`; boot smoke reads deploy .tab.
2. **KDesignation fields + rename** — add maps/flags, rename m_bDisplayFlag→m_bBynameDisplay
   (ripple KPlayer.cpp:1324). Verify: compile.
3. **SaveEndTimeInfo/LoadEndTimeInfo + Save/Load rewrite** — the DB drift.
   Verify: oracle-mirror roundtrip (empty→72B==old; with entries→+block; old-blob loads).
4. **Acquire{Prefix,Postfix}(int,time_t) + end-time maps + m_bAllowBroadcastAnnounceFix gate.**
   Verify: oracle-mirror logic (permanent vs timed, cap, dedupe).
5. **SetCurrentPrefix/Postfix + CanEquip + Equip/UnEquip + ResetCDTime** (buff + cooldown + independent).
   Reuse KPlayer m_BuffList / m_CoolDownList. Verify: build + boot.
6. **Activate() + wire into KPlayer::Activate.** Verify: build + boot; (optional) test_designation.lua acquire-timed→advance→expire.
7. **RemovePrefix/Postfix** rewrite (CD clear + UnEquip on current). Verify: build.
8. **Lua: 7 new + rename + REGISTER.** Verify: boot no-nil + test_designation.lua.
9. **Coverage-diff** close-out (nm surface vs implemented).

## 5. Remaining RE (minor, symmetric — decompile at port time if needed)
Postfix twins already inferred by symmetry: SetCurrentPostfix 081cda30, CanEquipPostfix 081cd878,
EquipPostfix 081cf002, UnEquipPostfix 081cf262, ResetPostfixCDTime 081ce0a4, GetPostfixEndTime
081cd664, AcquirePostfix 081cea26, RemovePostfix 081cf3f6. Also confirm exact `Activate()` caller
site in KPlayer and the cooldown-recipe manager index (DAT+0x2588) mapping to a 2010 member.

## 6. Defer / data-gap
Buff/CD/duration/Type columns are blank in leak data → those paths dormant (code ships, data absent).
Decide keep-or-drop the 3 removed Lua funcs vs script tree.
