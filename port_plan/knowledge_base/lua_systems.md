# JX3 Lua Content-Layer Atlas — Feature Systems + C++↔Lua Binding Surface

Source tree: stock deploy tree `scripts/` (GBK-encoded paths; surveyed with `LC_ALL=C` + `grep -a`).
Binding-side C++ read locally under `src/SO3World/Src/`.
Scope: top-down structural survey. The #1 port-failure mode (`GetXxx nil` / `Invalid newindex`) is a missing or mis-shaped C++ binding, so every system lists the concrete C++ symbols it depends on.

Two Lua-binding mechanisms are in play:
- **Global functions / const tables** injected once into the shared `KScriptCenter` Lua state (e.g. `GetPlayer`, `GetScene`, and the const enum tables `AI_ACTION`, `SKILL_KIND_TYPE`, …). The master const table is `g_LuaConstList[]` in `KLuaConstList.cpp`, injected by `KScriptCenter.cpp:724 piScript->RegisterConstList(g_LuaConstList)`.
- **Bound object classes** (Luna-style) whose methods/fields are declared with `DEFINE_LUA_CLASS_BEGIN/END` + `REGISTER_LUA_FUNC` / `REGISTER_LUA_INTEGER_READONLY` macros (`Luna.h`). Method literals therefore do **not** appear as plain strings in the source — grep for `REGISTER_LUA_FUNC(Class, Name)` instead of `"Name"`. Objects the scripts receive: `player` (KPlayer), `npc` (KNpc), `character` (KCharacter), `scene` (KScene), `skill` (KSkill), `ai` (KAILogic), `action` (KAIAction), `state` (KAIState).

---

## Top-level structure of `scripts/`

| Dir / file | English (pinyin) | Gameplay system |
|---|---|---|
| `ai/` | AI | **Core loop.** AI-editor flow-graph scripts (`StandardAI.lua`, `AIParam*.lua`, 290 files). Build-time descriptions compiled into C++ `KAILogic` at server boot. |
| `skill/` | Skill (jineng) | **Core loop.** Skill/buff definitions (30 subdirs, one per school 门派). Pure declarative → C++ skill engine. |
| `item/` | Item (wupin) | **Core loop.** Item `OnUse` behaviours, boxes, recipes, emotes. |
| `player/` | Player (wanjia) | **Core loop.** Player lifecycle: `PlayerScript.lua`, death, PVP, activation, message box, extra-points. |
| `map/` (→ symlink `Map`) | Map / scene (changjing) | **Core loop.** 189 scene dirs; each has `<scene>.lua` + `ai/` + `npc/` + `include/`. NPC spawn, scene logic, per-map AI params. Largest system. |
| `publicquest/` | Public quest (gongong renwu) | World/arena/battlefield public-quest framework. |
| `Activity/` | Activity (huodong) | 37 dirs — seasonal/operational events (spring festival, races, camp fights…). |
| `Tong/` | Guild (banghui) | Guild system: events, items, npc, traps, tech-tree. |
| `Craft/` | Crafting (zhizao) | Professions: forging, alchemy, cooking, gem, recipes. |
| `Domesticate/` | Taming (xunyang) | Pets / fellow-pets (`Domesticate.lua`, `FellowPet.lua`). |
| `Dungeon/` | Dungeon (fuben) | Dungeon custom functions + dungeon skill overrides. |
| `Mentor/` | Mentor (shitu) | Master/apprentice system, evoke, equip-score. |
| `Traffic/` | Traffic (jiaotong) | Mounts, flight paths, ferries, route nodes. |
| `achievement/` | Achievement (chengjiu) | Achievements + awards (`Achievement.lua`, `AchievementAward.lua`). |
| `RandomQuest/` | Random quest | Daily/random quest generators. |
| `operateactivity/` | Operations activity | Recharge-value operational activities. |
| `Emotion/` | Emote (biaoqing) | Social emotes (hug, cloak, magpie-bridge…). Mostly pure-Lua presentation. |
| `flowlib/` | Flow library | Event dispatch/filter framework (`event_dispatch.lua`, `system.lua`, `api.lua`). |
| `Include/` | Includes | Shared `.lh`/`.ls` headers, global strings, math, logic consts. |
| `LuaEnvInit/` | Lua env init | Editor-exported strings, global data, `LuaEnvInit.li`. Loaded at env bootstrap (recovered earlier per project memory). |
| `RemoteFromCenter/`, `RemoteFromClient/` | Remote entry points | RPC handler registration for center→GS and client→GS calls. |
| `script_server.lua` | Server master | Root include manifest + `RegisterAllFunction` — registers every client/center-callable remote function. |
| `server_main.lua` | Server main | `OnGameServerConnected` — BOT (broadcast-optimization) tuning. |
| `center_remote.lua`, `remote_for_gm.lua` | Center / GM remote | Center-side and GM remote-call surface. |
| `camp.lua` | Camp (zhenying) | Faction/camp logic. |

Pure-declarative `.tab`/`.ini` at root: `ScriptList.tab`, `ScriptName.ini`, `tree.lh`.

---

## STANDARDAI.lua `Setup` — ROOT-CAUSE ANALYSIS (current #1 port blocker)

**File:** `scripts/ai/StandardAI.lua` (631 lines). Header: `SOURCE : StandardAI.vsd` — this is an **AI-editor export**, not hand-written runtime code. It *describes* a flow graph that C++ compiles at boot.

### What `Setup` receives and where `action` comes from

`Setup(ai)` is **not** called by Lua. It is invoked from C++ at server startup by the AI loader:

- `KAIManager::Init()` (`KAIManager.cpp:15`) → `LoadAITabFile()` reads `SETTING_DIR/AIType.tab`; each row maps an `AIType` integer → a `ScriptFile` path (hashed to `dwScriptID`). Then (unless fast-boot) it calls `CreateAI(nAIType, dwScriptID)` for every row.
- `CreateAI` (`KAIManager.cpp:318`) `new`s a `KAILogic` and calls `pAI->Setup(nType, dwScriptID)`.
- `KAILogic::Setup(nType, dwScriptID)` (`KAILogic.cpp:28`): stores `m_nAIType = nType`, pushes **`this` (the KAILogic)** onto the Lua stack as the single argument, then `m_ScriptCenter.CallFunction(dwScriptID, "Setup", 0)`.

So **`ai` = the C++ `KAILogic` object.** Its Lua interface (`KAILogic.cpp:325-333`, `DEFINE_LUA_CLASS`):
- `ai.nAIType` — read-only integer. `REGISTER_LUA_INTEGER_READONLY(KAILogic, AIType)`; the macro **prepends `n`** (`Luna.h:139`), so the Lua field is exactly `nAIType` → `KAILogic::getAIType()` → `m_nAIType`. **No naming bug here** — `ai.nAIType` is correct.
- `ai.NewAction(id, key)` → `LuaNewAction` (`KAILogic.cpp:201`)
- `ai.NewState(id)` → `LuaNewState` (`:257`)
- `ai.SetInitState(id)` → `LuaSetInitState` (`:285`)
- `ai.RegisterUserAction(key, "FuncName")` → `LuaRegisterUserAction` (`:305`)

`action` is the **return value of `ai.NewAction(...)`**. On success `LuaNewAction` returns a bound `KAIAction` object (`pActionData->LuaGetObj(L)`, `:252`). On **any failure it hits `goto Exit0` with `nResult = 0`** — i.e. it returns **nothing → Lua `nil`**. The next line then does `action.SetParam(...)` → `attempt to index local 'action' (a nil value)`.

`Setup` body flow (line numbers = `StandardAI.lua`):
- `:17` `local param = g_AIParam[ai.nAIType]` — per-type tuning table loaded from `AIParam.lua` (`g_AIParam[1..N]`, ~18 fields each: AlertRange, EscapeLife, WanderRange, IsAutoTurn, …).
- `:21-25` `ai.RegisterUserAction(1001..1005, "...")` — registers 5 custom Lua actions (`CastBackStun`, `SetAlertEffect`, …, defined in `StandardAICustom.lua`).
- `:29` **FIRST** `action = ai.NewAction(772, AI_ACTION.CHECK_SKILL_FAILED_COUNTER)` — **this is where it dies.**
- `:30` `action.SetParam(3)` — the failing index.
- …~90 more `NewAction`/`SetParam`/`SetBranch` triples, then `:528+` `state = ai.NewState(id)` + `state.HandleEvent(AI_EVENT.X, actionId)`, ending `:626 ai.SetInitState(1)`.

### Why the first `NewAction` returns nil

`LuaNewAction` (`KAILogic.cpp:201-254`) guards, in order:
1. `:211` exactly 2 args.
2. `:214` `nActionID > KAI_ACTION_ID_NONE`.
3. `:217` **`nActionKey > eakInvalid`** — `nActionKey` = `AI_ACTION.CHECK_SKILL_FAILED_COUNTER`. If the `AI_ACTION` global table is absent/empty at call time, this is `nil`→`0` and the guard **fails → returns nil**.
4. `:219-228` if it's a system action, `m_AIManager.GetActionFunction(nActionKey)` must be non-NULL, else `"[AI] Unregistered sys action(ID, Key) in ai N"` → returns nil.

What I verified in the ported source:
- `AI_ACTION` and `AI_EVENT` **are** registered as global const tables (`KLuaConstList.cpp:2518-2519`, inside the unconditional region), and `CHECK_SKILL_FAILED_COUNTER → eakCheckSkillFailedCounter` exists (`KLuaConstList.cpp:1851`). The const list uses **symbolic `eak*` values**, so there is no possible value-vs-index drift *within* this source.
- The system action functions **are** registered — `KAIManager::RegisterActionFunctions()` (`KAIAction.cpp:2800`) includes `CheckSkillFailedCounter` (`:2855`) and the full set the script uses (SearchEnemy, CastSkill, SetPrimaryTimer, SelectState, ParamCompare, NpcStandardSkillSelector, …).

**Conclusion / hypothesis to verify at runtime:** in a correctly-built binary this path should succeed. The `action == nil` therefore points to one of, in priority order:
1. **`AI_ACTION` global not present in the Lua state when the AI scripts load** — i.e. `RegisterConstList(g_LuaConstList)` either did not run, ran *after* AI-script loading, or the AI script executes in a different/uninitialized Lua env. This is the most likely port/ordering gap and is cheap to check (dump `AI_ACTION` from Lua before Setup).
2. The `ai` object class registration (`DEFINE_LUA_CLASS(KAILogic)`) not active, so `ai.NewAction` itself is nil (would actually throw on `ai.NewAction(...)` call, not on `action.SetParam`, so **less likely** given the exact error text — the error on `action` implies `NewAction` *ran* and returned nil).
3. `GetActionFunction(eakCheckSkillFailedCounter)` returning NULL due to an **`eak*` enum-ordinal drift vs the v2.5.2 binary** (`m_ActionFunctionTable` indexed by a shifted key). Plausible given the project's history of enum drifts; check the server log for the exact `"[AI] Unregistered sys action"` vs a silent nil.

### The linked "AIType … is invalid" error

This is a **separate C++ log line**, emitted later and **caused by** the Setup failure above:
- `KAIVM::Setup(KCharacter*, int nAIType)` (`KAIVM.cpp:34`) runs when an NPC of that type spawns. At `:55` it calls `m_AIManager.GetAILogic(nAIType)`; if NULL → `:58 KGLogPrintf(KGLOG_ERR, "[AI] Setup AIVM failed, AIType %d is invalid.", nAIType)`.
- `GetAILogic` (`KAIManager.cpp:288`) returns NULL when the type isn't in `m_AITable` **or** its `pLogic` is NULL because `CreateAI` earlier failed — and `CreateAI` fails precisely when `KAILogic::Setup`'s Lua `Setup` call errored out (the `action`-nil throw). It logs `"[AI] Setup AI failed, AIType: N"` (`:340`).

So the two errors are one bug: **fix the first `NewAction` returning nil (most likely restore `AI_ACTION` const-table injection / load ordering), and the `AIType invalid` disappears.** What the AIType registration *expects*: a row in `SETTING_DIR/AIType.tab` (AIType, ScriptFile) **and** a `KALogic` that finished `Setup` without a Lua error.

---

## Core-loop systems — binding surface

### AI (`scripts/ai/`, `scripts/map/*/ai/`)
- **Purpose:** flow-graph descriptions of NPC behaviour (states + actions + event handlers). `StandardAI.lua` = generic mob template; `AIParam*.lua` (290 files) = per-map / per-scenario parameter+graph sets; also `GuarderAI`, `RangerAI`, `HoldRangerAI`, `DungeonStandardAI/BossAI`, `CommonFoolish`, `NpcPrison`. `StandardAICustom.lua`/`customFuctions.lua` hold the Lua bodies for user-actions (buff/alert/stun helpers). These are **build-time**; runtime execution is the C++ `KAILogic`/`KAIVM`.
- **`ai` (KAILogic) methods:** `NewAction`, `NewState`, `SetInitState`, `RegisterUserAction`; field `nAIType`.
- **`action` (KAIAction) methods:** `SetParam`, `SetBranch`, `GetParam` (used in custom fns).
- **`state` (KAIState) method:** `HandleEvent`.
- **Const tables required:** `AI_ACTION`, `AI_EVENT` (also `AI_EVENT_ACTION`, `AI_EVENT_CONDITION`, `AI_THREAT_TYPE`, `AI_TARGET_TYPE`, `AI_FILTER_TYPE`, `PREEMPTIVE_ATTACK`).
- **Custom-action bindings (on `character`/`npc`):** `GetTarget`, `SetTarget`, `GetAISelectSkill`, `AddBuff`, `DelBuff`, `GetCustomBoolean`, `SetCustomBoolean`, `PlaySound`; globals `IsPlayer`, `GetPlayer`, `GetNpc`, `Log`, `print`, `Include`, `FireAIEvent`.
- **Port-relevance:** **HIGH — current blocker.** Whole subsystem is C++ (`KAILogic`, `KAIVM`, `KAIManager`, `KAIAction`, `KAIState`, action-function table + `AI_ACTION`/`AI_EVENT` const injection). The Lua files are inert data; correctness depends entirely on binding + enum-ordinal parity with v2.5.2.

### Skill (`scripts/skill/`)
- **Purpose:** declarative skill + buff definitions, one subdir per school (门派). `Base/BaseKungfu*.lua` = weapon-type base kungfu; `Default.lua`, `MasterScript.lua`, `DynamicSkillGroupScript.lua`.
- **Bound object = `skill` (KSkill).** Top bindings: `BindBuff` (19.8k), `AddAttribute` (13.5k), `SetSubsectionSkill` (9.4k), `CanCast`, `OnSkillLevelUp`, `SetNormalCoolDown`/`SetPublicCoolDown`, `GetSkillLevelData`, `Apply`/`UnApply`, `AddSlowCheckSelfBuff`/`AddSlowCheckDestBuff`, `SetDisappearFrames`. Callbacks: `OnSkillLevelUp`, `Apply`, `UnApply`, `OnRemove`.
- **Const tables:** `SKILL_KIND_TYPE`, `SKILL_RESULT_TYPE`, `BUFF_COMPARE_FLAG`, `DIAMOND_SUB_TYPE`, `ATTRIBUTE_TYPE` (per project memory these were regen'd/fixed).
- **Port-relevance:** **HIGH.** Pure-declarative Lua but every symbol is a C++ `KSkill`/`KBuff` binding; project memory records prior fixes to `SetCheckCoolDown`, `OwnBuff`, `LuaBindBuff` arg-counts, `Sun/MoonSubsectionSkill`. Enum parity critical.

### Item (`scripts/item/`, plus per-map/craft/tong item dirs)
- **Purpose:** item use-behaviour. `OnUse(player, item)` handlers, boxes, `SkillRecipe.lua`, emotes, seasonal.
- **Bindings:** `AddItem`, `CostItem`, `GetItemAmount`, `SendSystemMessage`, `SwitchMap`, `OpenWindow`, `LearnSkill`, `AddTrainNoLimit`, `SetForceID`, `SetTimer`, `GetScene`, `GetEditorString`.
- **Port-relevance:** MEDIUM. Behaviour logic is Lua, but leans on inventory/skill/scene bindings. Mostly present.

### NPC / Map / Scene (`scripts/map/`)
- **Purpose:** 189 scene dirs (`10人英雄龙渊泽`, `苍山洱海`, `巴陵县`, …). Each: `<scene>.lua` (scene lifecycle), `npc/` (NPC dialogue/spawn/behaviour), `ai/` (per-scene AIParam graphs), `include/`.
- **Top bindings (dominant in tree):** `GetNpcByNickName` (38k), `GetScene` (24k), `CreateNpc` (7.8k), `DoAction`, `Say`, `GetPlayer`, `GetNpc`, `OpenWindow`, `SendSystemMessage`, `RemoteCallToClient`, `SetPosition`, `GetKungfuMount`, `FxLog`. NPC object methods: `Say`, `DoAction`, `SetTarget`, `AddBuff`, `CanDialog`, `OnDialogue`, `SetPosition`.
- **Const tables:** `TARGET`, `CHARACTER_ACTION_TYPE`, `CHARACTER_OTACTION_TYPE`, `SWITCH_MAP`.
- **Port-relevance:** HIGH breadth. Scene/NPC creation + AI attach is the core spawn path; ties directly to the AI blocker (each `CreateNpc` sets an `AIType` → `KAIVM::Setup`). Project memory notes the map-drop subsystem was tolerantly skipped.

### Player (`scripts/player/`)
- **Purpose:** player lifecycle — `PlayerScript.lua`, `PlayerDeath.lua`, `PVPScript.lua`, `ClientPlayerActivate.lua`, `PlayerMessageBox.lua`, `PlayerAutoCastFormation.lua`, `PlayerExtPoint.lua`, `PlayerPresentCode.lua`.
- **Bindings:** `LearnSkill`, `AddItem`, `OpenRouteNode`, `SetMapVisitFlag`, `SendSystemMail`, `SendClientErrorMsg`, `AddBuff`/`GetBuff`, `ForceFinishQuest`, `DateToTime`.
- **Port-relevance:** HIGH (core loop) but bindings are broadly present; this is the entry surface for most client→server remote calls (`script_server.lua RegisterAllFunction`).

---

## Support systems — binding surface (lower port priority)

- **Public quest** (`publicquest/`): `GetPQ`, `GetPQValue`, `SetStatistics`, `GetStatisticsTable`, `GetPQIDTable`, `AwardFormula`, `AcquireAchievement`, `SendMessage`. World/arena/battlefield framework. MEDIUM.
- **Tong / guild** (`Tong/`): `OpenWindow`, `RemoteCallToCenter`, `ChangeCampCost`, `GetBoxSize`, `CanDialog`, `OnDialogue`. Center-coupled; project memory flags corps/tong shop drift as deferred. MEDIUM.
- **Craft** (`Craft/`): `GetEditorString` (1.5k), `CostItem`, `GetItemAmount`, `OnUse`, `RemoteCallToClient`, `GetBuff`. MEDIUM.
- **Achievement** (`achievement/`): `IsBookMemorized`, `IsReputeAtLevel`, `AddAchievementCount`, `AcquireAchievement`, `GetTotalEquipScore`, `IsEquipedSpecialPackage`. Query-heavy on player state. LOW/MEDIUM.
- **Domesticate / pets** (`Domesticate/`): `GetDomesticate`, `IsFellowPetAcquired`, `GetCubItemInfo`, `AddBuff`, `RemoteCallToClient`. Project memory has a `KDomesticate` port note. MEDIUM.
- **Traffic** (`Traffic/`): `OpenRouteNode`, `IsOpenRouteNode`, `AutoFly`, `CreateNpc`, `GetAutoDialogString`. LOW.
- **Mentor** (`Mentor/`): evoke/remote split (`Evoke_from_center/client`, `remote_from_*`), equip-score. Center-coupled. LOW/MEDIUM.
- **Activity / operateactivity / RandomQuest**: event schedulers; heavy `GetEditorString` + remote calls; mostly self-contained Lua orchestration over existing bindings. LOW.

---

## Pure-Lua vs C++-binding-dependent

**Effectively pure-Lua** (orchestration/data, no new C++ port beyond already-present primitives): `Emotion/` (presentation emotes), `flowlib/` (event dispatch framework — pure Lua library), `Include/` (headers/strings/math consts), `RandomQuest/`, most of `Activity/`/`operateactivity/` (compose existing calls), and the AI `AIParam*.lua` data files themselves.

**C++-binding-dependent (port risk lives here):**
- **AI** — entire runtime is C++ (`KAILogic`/`KAIVM`/`KAIManager`/`KAIAction`/`KAIState`) + `AI_ACTION`/`AI_EVENT` const injection. **Current blocker.**
- **Skill/Buff** — every `skill.*` call is a `KSkill`/`KBuff` binding; enum parity (`ATTRIBUTE_TYPE`, `SKILL_KIND_TYPE`, `DIAMOND_SUB_TYPE`) critical.
- **NPC/Scene** — `CreateNpc`/`GetScene`/`GetNpcByNickName` and the NPC object model; the spawn path that triggers `KAIVM::Setup`.
- **Item/Player/Quest** — inventory, quest-value, mail, remote-call, mount bindings; broadly present but individually gap-prone (the classic `GetXxx nil` failure).

Key binding-surface globals seen tree-wide (injected functions, not object methods): `GetPlayer`, `GetNpc`, `GetNpcByNickName`, `GetScene`, `CreateNpc`, `IsPlayer`, `GetEditorString`, `GetAutoDialogString`, `SendSystemMessage`, `SendSystemMail`, `RemoteCallToClient`, `RemoteCallToCenter`, `SwitchMap`, `OpenWindow`, `SetTimer`, `Random`, `Log`, `FxLog`, `FireAIEvent`, `Include`.
