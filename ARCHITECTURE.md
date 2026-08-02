# JX3 Server Architecture — Build and Integration Map (2010 Source / v2.5 Binary)

This overview is intended to make the build and connection model of a
GameServer/GameCenter/Gateway immediately understandable, and to identify every
binding point that a subsystem must use. It exists to prevent silent omissions
such as the earlier KExterior Lua-registration failure. Sources: the 2010 source
in `linux-build/src` and `include`, DWARF in `jx3_dwarf/`, and original developer
design documents in `source/JX3-AIO/Document/`. Detailed file-and-line evidence
is in `docs/arch_research/{devdocs,build_init,bindings}.md`. Port case studies
are in `docs/{exterior,hair}_port/`.

---

## 1. Four processes and the connection flow

```
                 (account + password)
   Client  ───────────────────────────►  Bishop / Gateway ──► Paysys (points / gold)
     │                                         │ auth succeeds; directs client to GS
     │   gameplay (C2G/G2C protocol)           ▼
     └──────────────────────────────►  GameServer (KSO3World)  ◄──── each GS owns a map shard
                                              │  ▲                    (multiple GS instances = scenes)
                            (KS2R/KR2S)       ▼  │ (role-data blob through KRelayClient)
                                        Relay / GameCenter  ──►  Database (MySQL)
                                        global control + relay: player/map-instance/
                                        friend/guild/task-variable data; login and cross-server control
                                              │
                                        RelayServer / LogClient
```

- **Gateway (Bishop)** authenticates an account and password. On success, the
  client **reconnects directly to the GameServer**. Gateway does not relay each
  gameplay packet. (In the leak, Gateway is binary-only; see
  `[[jx3-auth-paysys]]`.)
- **GameServer** is the main process. It contains `KSO3World` and executes all
  gameplay logic for the scenes it owns. Client ↔ GS traffic is direct
  (`GS_CLIENT` / `CLIENT_GS` protocol).
- **GameCenter / Relay** is the global controller (总控) and relay (中转). It
  keeps global state—online players, map instances, friends, guilds, and task
  variables—coordinates **login** and **cross-server** flows, and proxies the
  database. Role-data `Save`/`Load` blobs travel as `KS2R`/`KR2S` protocol through
  center to MySQL. GameServer **never** communicates with the database directly;
  persistence always goes through `KRelayClient`.
- Handshake chain: `KGatewayClient::ConnectGateway/DoHandshakeRequest`
  (client → gateway), gateway directs client → GS,
  `KRelayClient::Connect/DoHandshakeRequest` (GS → center). See
  `[[jx3-lan-multiplayer-blocker]]` for the loopback-IP trap on LAN deployments.

---

## 2. SO3GameServer init chain: what constructs what

```
main()  (Main.cpp:94)
  └─ log → memory pool → clock
  └─ g_SO3GameServer.Init()                         (KSO3GameServer.cpp:16)
       ├─ g_pSO3World = new KSO3World
       ├─ g_pSO3World->Init(piFactory)              (KSO3World.cpp:71) ── DATA-LOAD CORE
       │     ├─ recorder/clock → reads gs_settings.ini → TeamServer → FellowshipMgr
       │     ├─ m_Settings (KWorldSettings::Init)    ◄── 25 config-table managers (section 3)
       │     ├─ KScriptCenter (Lua environment) + InitAttributeFunctions
       │     ├─ Item / Shop / Profession / Road / AI / Buff / Skill / Drop / Camp / PQ managers
       │     ├─ TransmissionList → StatDataServer → object-index SetPrefix
       │     └─ CenterRemote/ScriptServer Setup → lzo_init
       └─ ONLY THEN opens networking: g_RelayClient → g_PlayerServer → m_Eyes → g_LogClient
  └─ Run() loop → g_pSO3World->Activate() every frame (traverses Player, then Scene)
```

**Initialization invariant:** tables/settings come **before** managers;
managers come **before** objects; scenes come **before** NPCs/players; and the
network opens **only after all data loading has completed**. `UnInit` must run in
the reverse order, guarded by its `bXxxInitFlag` flags. The boot milestone for
the entire pipeline is the log line **`Load game settings ... [OK]`**. Every port
must retain that milestone as a no-regression check.

### Taxonomy: core, sub-core, per-player, and per-scene

- **Core (root):** `KSO3World` (`g_pSO3World`) owns all managers, the
  `KObjectIndex` ID→object registry, and all `KScene` instances.
- **Sub-core:** `KWorldSettings` (config tables), `KScriptCenter` (Lua
  environment), and global `*Manager` instances (AI, Buff, Skill, Drop, Shop,
  Road, and so on). They are initialized in the chain above.
- **Object inheritance tree:**
  `KBaseObject → { KItem, KSceneObject → { KCharacter → { KNpc, KPlayer }, KDoodad } }`.
  `KCharacter` supplies the base state machine and combat state (`m_BuffList`,
  `m_ThreatList`, `m_AIVM`, and target selection).
- **Per-player:** `KPlayer` (section 3). **Per-scene:** `KNpc` and `KDoodad`
  live in a `KScene` grid of `KRegion` / `KCell`.

---

## 3. The two subsystem attachment axes: KWorldSettings and KPlayer

These are the two places where most subsystems attach. Porting a feature almost
always touches one or both.

### 3a. KWorldSettings: 25 configuration-table managers

These managers are embedded by value and initialized in sequence by
`KWorldSettings::Init` (`KWorldSettings.cpp:8`), from `m_ConstList` (`:32`) to
`m_GameCardInfoList` (`:124`). Each manager implements “read a `.tab` file →
provide a lookup map.” Examples already ported include `m_Exterior` (`:117`,
`KExterior`) and `m_HairShop` (`:121`, `KHairShop`). **Adding static subsystem
configuration means adding a member here and calling `m_Xxx.Init()`.**

### 3b. KPlayer: approximately 20 business sub-objects

`class KPlayer : public KCharacter`. Important sub-objects include `m_ItemList`,
`m_SkillList`, `m_QuestList`, `m_Designation`, `m_ExteriorBox`, `m_HairBox`,
`m_Achievement`, `m_PK`, and `m_AntiFarmer`, plus the inherited `KCharacter`
state (`m_BuffList`, `m_ThreatList`, `m_AIVM`, `m_SelectTarget`). **Adding
per-player state means embedding a member in `KPlayer.h`, initializing it with
`Init(this)`, and adding a role block if it is persisted.**

---

## 4. The C++ ↔ Lua binding boundary

Lua content can be changed without recompilation. It calls a C++ `LuaXxx`
wrapper, which unpacks the Lua stack and invokes the real C++ method. C++ owns
data, network, database, hot paths, and security. A missing binding produces a
runtime `GetXxx nil` error: the script has a doorway, but C++ has not installed
the door. There are **three binding forms**; choosing the correct form is where
KExterior previously failed:

| Form | Mechanism | Use when | Registration location |
|---|---|---|---|
| **(a) `Luna<T>` object** | `DECLARE_LUA_CLASS(T)` generates `LuaGetObj`, `szClassName`, and `LuaInterface[]`; `Register` creates the `__index` / `__newindex` metatable. Script: `GetT():Method()` | A whole object is exposed: per-instance (`KPlayer`, `KItem`) or a **singleton** (`KHairShop`, `KMentorCache`) | `Luna<T>::Register` in `KScriptCenter.cpp:729-780` (`#ifdef _SERVER` for server-only), plus free function `LuaGetT` and `{"GetT", ...}` in `KBaseFuncList.cpp` |
| **(b) free function `GetXxx`** | Global function in the base-function table | A global function, or a gateway exposing a singleton to Lua | `KBaseFuncList.cpp:4833-5117` (179 entries) |
| **(c) `REGISTER_LUA_FUNC(KPlayer/KCharacter, X)`** | A method attached to the current player | Operations on the current player | `KLuaPlayer.cpp` (407 registrations, including `KCharacter` / `KSceneObject` methods) |

**Server-build counts:** 37 registered `Luna<T>` classes (31 in the server
build), 179 base-function entries, 407 `REGISTER_LUA_FUNC` registrations on
`KPlayer`, and 23 role blocks. **How to identify the form from the binary:** if
v246 `LuaGetObj` invokes `lua_newuserdata + setmetatable "KClass"`, it is a
`Luna<KClass>` object, form (a)—**do not** force it into a KPlayer method.
KExterior was incorrectly moved from (a) to (c), leaving six query bindings nil;
see TODO §1.

---

## 5. Persistence: role blocks and KRelayClient

- Per-player state is stored in **role blocks**. Enum `ROLE_DATA_BLOCK_TYPE`
  (`include/Include/KRoleDBDataDef.h:14-44`) defines 23 valid blocks,
  `rbtSkillList` through `rbtHairBoxData`.
- Save uses `SAVE_ROLE_BLOCK(m_Xxx.Save, rbtXxx, 0)` in `KPlayer::Save`
  (`KPlayer.cpp:2364`). Load uses `case rbtXxx:` in the switch
  (`KPlayer.cpp:2040`) to call `m_Xxx.Load`.
- **Append a new `rbtXxx` before `rbtTotal`; never insert one in the middle.**
  Its value is a database tag, and shifting it corrupts every other system’s
  blobs. The 2010 `KRoleBlockHeader` has no chunk key, so data must be combined
  into one block (v246 uses separate chunks).
- Database structs use `#pragma pack(1)`. Pin their byte layout from DWARF and
  verify a round trip. Blobs pass through **`KRelayClient`** (`KS2R` / `KR2S`) →
  center → MySQL. GameServer does not access the database directly.

---

## 6. Network: six protocol families

`C2G/G2C` (client ↔ gateway), `GS_CLIENT/CLIENT_GS` (client ↔ GameServer
gameplay), `G2R/R2G` (gateway ↔ relay), `KS2R/KR2S` (GameServer ↔ center,
including database blobs), `EDITOR_2_GS/GS_2_EDITOR`, plus Bishop/Paysys/Log.

- A client-facing packet ID belongs in `GS_CLIENT_PROTOCOL` and is emitted by
  `KPlayerServer::DoSyncXxx` (`KPlayerServer.cpp:3400`), called by **every
  setter** that changes state the client must see.
- An internal packet uses `KS2R/KR2S_PROTOCOL` plus
  `REGISTER_INTERNAL_FUNC(name, handler, SIZE)`. `SIZE` is the fixed struct
  size; a byte mismatch rejects a handshake. See saga #22
  `[[jx3-linux-build-recovery]]`.

---

## 7. Player lifecycle: login, cross-server, synchronization

- **Client connection states:** `gsInvalid → gsWaitForGUID → gsSyncData →
  gsPlaying`, plus `gsSearchMap` / `gsTransferData` during map changes.
  GameCenter uses `rs*` states (`rsOffline` / `rsOnline` and `rsLG_*` / `rsCG_*`
  for login and cross-server).
- **Login:** Bishop authenticates → Relay `psLogin` → GS approves → client
  reconnects directly to GS → synchronization → `psOnline`.
- **Cross-server:** two cases exist: within one GS (lightweight) and **across
  GS instances** (full character transfer Source-GS → Relay → Destination-GS;
  client reconnects to destination with a new GUID).
- **Synchronization strategy (剑三同步策略):** synchronize the **nine regions**
  surrounding the player (a Region is 16m; cells are 1m × 1m), targeting ≤2s.
  It distinguishes logical data (coordinates, speed, state, HP percentage—often
  synchronized) from display data (rarely changed). Movement is
  **client-authoritative with server validation and correction**, using position
  history and frame rollback (“the server frame number is authoritative”) to
  prevent pull-back and teleport hacks. New objects use broadcast plus a
  **forced synchronization for two seconds** for passive players.

---

## 8. ✅ Subsystem integration checklist: do not omit bindings during a port

These are every attachment point a **new** subsystem may require. Skipping one
can leave a feature silently non-functional even when compilation and boot pass.
Compare with `docs/hair_port/INTEGRATION.md`, a fully completed example.

**A. Configuration (when static `.tab` data exists)**

1. `KXxx::Init()` loads the `.tab`; add a member to `KWorldSettings`, call
   `m_Xxx.Init()` (`KWorldSettings.cpp:~117`), and add the symmetric `UnInit()`.

**B. Per-player state (when state belongs to a player)**

2. Embed the member in `KPlayer.h` (**do not** hard-code an offset; let the
   compiler lay it out).
3. Call `m_Xxx.Init(this)` beside its siblings in KPlayer initialization and set
   the player back-pointer.

**C. Persistence (when state is stored in the database)**

4. Add enum `rbtXxxData` **before** `rbtTotal` (`KRoleDBDataDef.h`).
5. Add `SAVE_ROLE_BLOCK(m_Xxx.Save, rbtXxxData, 0)` (`KPlayer.cpp:2364`).
6. Add `case rbtXxxData:` to the load switch (`KPlayer.cpp:2040`) and call
   `m_Xxx.Load`.
7. Use `#pragma pack(1)` for the database struct, add a static size assertion,
   and verify byte-exact Save ↔ Load round trips.

**D. Lua binding: choose one form (the KExterior omission occurred here)**

8. Determine the form from the binary (`Luna<T>` object versus KPlayer method).
9. For a form-(a) singleton `Luna<T>`: put `DECLARE_LUA_CLASS(T)` in the header
   (`#ifdef _SERVER` plus `#include "Luna.h"`); put
   `DEFINE_LUA_CLASS_BEGIN(T) REGISTER_LUA_FUNC(T, M) ... END` in `KLuaT.cpp`;
   add `Luna<T>::Register` to `KScriptCenter.cpp`; add free function `LuaGetT`
   and `{"GetT", ...}` to `KBaseFuncList.cpp`.
10. For a form-(c) player method: add `LuaXxx` on KPlayer and
    `REGISTER_LUA_FUNC(KPlayer, Xxx)` in `KLuaPlayer.cpp`.
11. **Verify that the binding is live:** boot without nil errors and, where
    possible, execute `test_xxx.lua`.

**E. Packet synchronization (when the client must observe the change)**

12. Client-facing: add the ID to `GS_CLIENT_PROTOCOL` and call `DoSyncXxx` from
    every setter.
13. Internal: add `KS2R/KR2S` plus `REGISTER_INTERNAL_FUNC(name, handler, SIZE)`
    with the exact `SIZE`.

**F. Logging (optional)**

14. Add `KLogClient::LogPlayerXxx` (it may be stubbed).

**G. Apply and hook into existing mechanisms**

15. Reuse the 2010 mechanism instead of transcribing a v246 offset. For example,
    appearance should use `KItemList::SetRepresentID` plus enum
    `PLAYER_EQUIP_REPRESENT`, **not** write directly to `KPlayer+0x95a8`.
    Lock drift with `typedef char _CHK[(COND)?1:-1]`.

**H. General discipline**

16. Back up before editing; port small-to-large; compile and boot each slice;
    perform a final coverage diff (gap == deferred list).
17. Record version gaps: a new subsystem may only render on the v246 client, not
    PAP2/2010, so its packet may be deferred.

---

## 9. Detailed references

- `docs/arch_research/devdocs.md`: original developer design documents (SO3World
  taxonomy, synchronization, login/cross-server, combat/AI/quest/enchant/
  magic-attribute systems).
- `docs/arch_research/build_init.md`: init chain, 25 KWorldSettings managers,
  KPlayer sub-objects, and taxonomy with file-and-line evidence.
- `docs/arch_research/bindings.md`: the three Lua binding forms, counts,
  networking, and the original checklist with file-and-line evidence.
- [`port_plan/`](../port_plan/README.md): Feature Atlas, method inventory, and
  the Feature Card contract for the v2.5 port.
- `docs/{exterior,hair}_port/`: two complete port case studies (hair is the
  DWARF-based model).
