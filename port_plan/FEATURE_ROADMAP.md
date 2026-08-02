# JX3 Feature Roadmap

This is the public, evidence-led roadmap and operating protocol for deciding
which **Feature Card** and later which **Wave** should be created next. It is
intentionally not a method checklist or a statement that any listed capability
has already been ported.

The target is v2.5.2. Target DWARF/binary and target runtime behavior define
the contract; the 2010 source is the implementation baseline. See
[README.md](README.md) for authority rules and
[layers/README.md](layers/README.md) for the measurement layers.

## How to use this roadmap

```text
Roadmap entry
  = known capability and its dependency/evidence leads
  = not yet a task and not an acceptance claim

Feature Card
  = bounded contract for one capability, created only when that entry is ready
  = source of truth for research, implementation, review, and acceptance

Wave
  = created just in time from a small set of accepted-or-ready Feature Cards
  = has one integrated player/system scenario
```

Do not create empty cards for every row below. Do not create a Wave merely
because a roadmap band exists. A future capability becomes a Feature Card only
when its outcome, prerequisite contracts, target evidence roots, and acceptance
scenario can be stated without inventing target behavior.

## Operating model: roadmap, Wave, and Feature Card

These three artifacts answer different questions and must not duplicate each
other's state.

| Artifact | One source of truth for | Must not contain |
|---|---|---|
| This roadmap | What capabilities are known, their dependency graph, likely evidence leads, and the next planning frontier. | Raw diffs, implementation verdicts, or an acceptance claim. |
| Feature Card | The complete, bounded contract for one capability: evidence, scope, owner, review, build, harness, and verdict. | A whole-release promise or unrelated backlog. |
| Wave manifest | A just-in-time release milestone: the required card IDs and one integrated scenario that joins them. | Per-method research, a second copy of card checklists, or a generic “mostly done” status. |

The state flow is strictly one-way in meaning, although a finding can return a
card to research:

```text
roadmap entry
  -- enough target evidence to define a bounded contract --> Feature Card
  -- small set of card contracts form one scenario ------> Wave manifest
  -- all required cards accepted + integrated pass ------> accepted Wave
```

An implementation may never promote a roadmap row directly to a Wave. A
historical port note may never promote a row or card to `accepted` without a
current evidence/review/build/oracle record.

### Directory and naming convention

When the first cards and Waves are created, use this layout:

```text
port_plan/
  FEATURE_ROADMAP.md                  # this file; capability graph and protocol
  port-card.tmpl                      # mandatory card template
  waves/
    <WAVE-ID>/
      WAVE.md                         # one Wave manifest
      STATE.md                        # private execution ledger for that Wave
      cards/
        <FEATURE-ID>.md               # one detailed Feature Card
      evidence/                       # local dossiers, captures, manifests, and backups
  FEATURE_INDEX.generated.md          # optional generated navigation only
```

IDs are stable, uppercase, and capability-oriented:

```text
FND-CLIENT-GS-SECURE-TRANSPORT
PLY-ENTER-WORLD
NPC-SPAWN-AND-STANDARD-AI
COMBAT-CAST-ONE-SKILL
```

Never use a file name, class name, or a raw protocol ID as a Feature ID unless
that object itself is the observable capability. A Wave ID is sequential only
after it is opened, for example `W1-ENTER-WORLD`; roadmap bands `T0`–`T7` are
not Wave IDs.

### Required roadmap-entry format

Every catalogue row added to this document must use this information, whether
it is rendered as the tables below or generated later:

| Field | Required meaning |
|---|---|
| `ID` | Stable future Feature Card ID. |
| `Capability` | One player-visible or system-observable outcome, stated without naming files to edit. |
| `Depends on` | Capability IDs or named contracts that must be accepted before an implementation claim. |
| `Unlocks` | The next capabilities that can safely consume this result. |
| `Evidence leads` | Initial target functions/types/protocols/tables/scripts and relevant layers; they are leads, not proof. |
| `Status` | Roadmap lifecycle only: `catalogued`, `research-ready`, `card-open`, `historical-revalidation`, or `blocked-by-foundation`. It never claims card execution completion. |

When a row needs a material scope decision, add a short note immediately below
the table identifying its state transition and likely boundary set. Do not add
an implementation checklist here; that belongs in the card.

### Card slicing rules

A card is neither a source-file slice nor a tiny transition fragment. Create
one card when all of the following are true:

1. It has one outcome that a player, client, process, or persistence oracle can
   observe.
2. It has one bounded state transition, for example `role selected -> scene
   ready`, `NPC alive -> dead with loot`, or `currency before -> purchase after`.
3. Its target evidence closure can be named, including any protocol/layout/Lua/
   table/persistence boundary that lies on that transition.
4. It has a plausible bounded acceptance scenario; static-only work must state
   why it has no behavior claim.

Keep internal steps inside the same card when they have no independent outcome.
For example, character selection, ready confirmation, and initial object sync
belong to `PLY-ENTER-WORLD`, not three cards. Split a card only when at least
one condition holds:

- the two outcomes have independent scenarios and can be accepted separately;
- they use different shared contracts whose failure domains must be isolated;
- one outcome is a reusable foundation needed by several later cards; or
- the combined closure cannot be owned/reviewed/built as one bounded change.

Shared spines are the deliberate exception. A role envelope, secure transport,
or save/load version contract may deserve a foundation card even though it is
not a player-facing feature, because many player outcomes depend on the same
contract and it has its own oracle.

### Feature Card creation protocol

Create `waves/<WAVE-ID>/cards/<FEATURE-ID>.md` by copying
[port-card.tmpl](port-card.tmpl), preserving every section. Convert the YAML
block at the top of the template to YAML front matter before the first real card
is created, so that the generated index can parse it. The required front matter
is:

```yaml
---
feature_id: "<DOMAIN-OUTCOME>"
title: "<player/system-observable capability>"
wave_id: "<WAVE-ID>"
status: "researching"
priority: "<P0|P1|P2>"
kind: "<drift|new|foundation|integration>"
owner: "<orchestrator/producer>"
producer_model: "<codex|claude>"
reviewer_model: "<claude|codex>"
graph_db_sha256: "<sha256 or pending>"
target_artifacts: []
evidence_package: "<saved evidence bundle path or pending>"
harness_contract: "<path or N/A — static-only card>"
candidate_baseline: "<git SHA + build/input manifest>"
created_utc: "<UTC>"
updated_utc: "<UTC>"
next_action: "<one executable action>"
---
```

The orchestrator must fill the remaining card sections in the following order;
do not jump to implementation scope.

1. **Outcome and boundary** — capability, observable results, non-goals,
   dependency cards, state transition, consumers.
2. **Target contract** — target artifact SHA, target roots, and target-backed
   invariants. Mark an absent boundary `N/A — <reason>`; never silently omit it.
3. **Origin comparison** — saved GraphEngine queries, target/source/candidate
   observations kept separate, fact/inference/unresolved classification, and
   an explicit unknown policy.
4. **Boundary matrix** — decide which of C++ logic, ABI/layout, Lua, table,
   protocol/transport, persistence, and build/DSO applies. A used boundary
   needs a verification method.
5. **Implementation lease** — exact baseline, allowed writes, forbidden paths,
   expected target-entity disposition, overlay and backup paths.
6. **Acceptance** — cross-model review, build provenance, static checks, and
   harness contract. A behavioral card requires stock/candidate arms; it cannot
   substitute a clean boot or TCP accept.
7. **Handoff** — hashes, evidence bundle, review verdict, runtime bundle,
   rollback, next action, and the four-part worker report.

The card state advances only at these checkpoints:

```text
draft
  -> researching       : outcome and evidence leads are recorded
  -> contract-ready    : target closure, unknown policy, boundaries, harness,
                         baseline, and ownership are complete
  -> implementing      : producer worktree/lease is issued
  -> reviewing         : raw diff and producer report exist
  -> verifying         : opposite-model review passed and required build/oracle runs
  -> accepted          : every required checklist item passes

any state -> blocked   : blocking unknown + owner + resolution evidence + next action
any state -> deferred  : safely off-path + owning future capability
```

`accepted` has a strict meaning: all required checklist boxes are complete;
target evidence is reopenable; no required-path unknown remains; producer and
accepting reviewer are different model families; source/build/active-binary
provenance is recorded; the declared harness passes; rollback remains available.
An agent must stop and return the card to the appropriate owner when one of
these conditions is absent.

### Wave manifest creation protocol

Create a Wave only when a contiguous roadmap frontier has enough
`contract-ready` cards to define one integrated scenario. The Wave manifest is
stored at `waves/<WAVE-ID>/WAVE.md` and uses this exact format:

````md
# Wave <N>: <player/system milestone>

```yaml
---
wave_id: "W<N>-<SHORT-NAME>"
status: "planned" # planned | active | blocked | verifying | complete
roadmap_band: "T<N>"
baseline: "<accepted git SHA + manifest>"
integrated_scenario: "<one observable journey>"
created_utc: "<UTC>"
updated_utc: "<UTC>"
next_action: "<one executable action>"
---
```

## Entry contract

- <accepted prerequisite Feature Card ID + required result>

## Required Feature Cards

| ID | Capability | Card status | Why required for this scenario |
|---|---|---|---|
| `<FEATURE-ID>` | <outcome> | `<contract-ready|implementing|...>` | <dependency> |

## Integrated scenario and exit contract

```text
Preconditions: <data/config/role seed>
Input: <player/system action sequence>
Expected result: <bounded observable result>
Must stay unchanged: <named invariants>
```

Wave accepted iff every Required Feature Card is `accepted` and this integrated
scenario passes with recorded provenance. Deferred cards are not listed here.

## Blockers and next frontier

| Blocker | Owning card | Resolution evidence | Effect on roadmap |
|---|---|---|---|
| <item> | `<FEATURE-ID>` | <evidence> | <do not open / return to roadmap band> |
````

The Wave manifest references cards; it never copies their evidence matrices or
checklists. It is `blocked` if a required card is blocked, and it cannot be
`accepted` while a required card is merely `deferred`, `reviewing`, or
`verifying`.

### Creation and execution order

Use this repeatable sequence for every frontier:

1. Refresh only the inventory/layer/Graph evidence relevant to the candidate
   capability.
2. Update this roadmap if the evidence changes its dependency, outcome, or
   readiness; do not create a card yet if the target contract is still vague.
3. Create the detailed Feature Card and pass its `contract-ready` review.
4. When two to five cards share one stable integrated scenario, create the Wave
   manifest that references them.
5. Run the agent-team workflow per Feature Card: evidence analyst -> producer
   -> opposite-model reviewer -> runtime verifier. Keep worktree ownership and
   baseline manifests card-scoped.
6. After each card result, update its card first, then this roadmap's status
   and dependency frontier. Do not infer a Wave result from a card result.
7. Run the integrated Wave scenario only after every required card is accepted;
   then update the Wave manifest and open the next frontier.

### Runtime localization is allowed before card acceptance

The ordering above governs **acceptance**, not diagnosis.  When a player
journey fails, the coordinator must be able to run a reversible paired
stock/candidate expedition before every foundation card is accepted.  Its sole
purpose is to locate the first divergent observable transition; it neither
promotes a card nor bypasses the prerequisite rule for implementation.

The expedition records one stock arm and one candidate arm with the same
topology, seed, active-binary/config/data identities, raw capture/log, last
shared milestone, and first divergent milestone.  It is the priority source
for the next evidence closure.  Static research then covers only that boundary
and its immediate producer/consumer/ABI closure.  Do not pre-emptively build a
full route-to-scene dossier or a multi-card overlay.

For an observed divergence, the delivery loop is:

```text
paired probe -> first divergence -> minimum target closure -> vertical fault slice
-> opposite-model review -> build -> paired probe
```

The loop repeats at the next boundary.  A probe can conclude `shared`,
`divergent`, or `oracle unavailable`; only the latter may block the card, and
it must name the exact missing input.  Two static passes that do not alter the
next probe or patch are a stop signal, not a reason to enlarge the dossier.

## Evidence basis and limits

The current machine inventory contains **1,716** target-minus-candidate method
rows: **1,581** hard missing-name candidates and **135** uncertain
name-elsewhere candidates. The largest clusters are player hub (590 rows),
network infrastructure (244), skill/script (172), cosmetic (135), and
world/scene (101).

Those counts are navigation evidence only. A method can serve several features,
and a feature may require methods that are already present but semantically
drifted. The roadmap therefore groups methods by observable capability and
state transition, never by source file or raw row count.

## Status vocabulary

| Status | Meaning |
|---|---|
| `catalogued` | Capability, dependencies, and likely evidence leads are known; no Feature Card exists. |
| `research-ready` | Target roots can be named from the current inventory/knowledge base; a card may be opened for evidence collection. |
| `card-open` | A Feature Card exists and records the current research/blocked state; it has no acceptance claim. |
| `historical-revalidation` | Earlier work is documented, but no current Feature Card evidence grants acceptance. Revalidate before relying on it. |
| `blocked-by-foundation` | Do not open an implementation card until the named prerequisite is accepted. |

No roadmap status means `accepted`, `implemented`, or runtime-correct.

## Dependency timeline

The bands give the default critical path. They are not future Wave numbers;
independent research may proceed in parallel, but implementation follows
accepted prerequisites.

```text
T0  planning control and evidence refresh
 |
T1  runtime initialization -> secure client/GS transport -> role load spine
 |
T2  enter world -> movement and standard NPC/AI startup
 |
T3  one skill cast -> NPC death -> loot in inventory
 |
T4  save/logout/relogin -> item use/equipment -> skill/quest progression
 |
T5  currency/shop and cosmetic journeys -> pet/talent/reputation/activity
 |
T6  cross-GS/social journeys -> residual operational/content systems
 |
T7  repeatability, reconnect, malformed-input, and completeness hardening
```

## Capability catalogue

`Depends on` names capabilities, not source files. `Evidence leads` are the
first places to inspect; each eventual card still needs its own GraphEngine
dossier and raw target evidence.

### T0 — Planning control

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `CTRL-FEATURE-TRACEABILITY` | Keep inventory, layer outputs, Feature Cards, and later Wave status traceable without duplicating acceptance state. | — | Every port decision | `PORT_MAP.csv`, L1–L7, card template | research-ready |

### T1 — Runtime and entry contracts

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | Rebuilt GameServer initializes required data, scripts, DSOs, locale, and map/config inputs with a target-bounded boot fingerprint. | — | All runtime features | `KSO3World`, `KWorldSettings`, `KNpcTemplateList`, loader/table L3 | card-open |
| `FND-CLIENT-GS-SECURE-TRANSPORT` | Client and GameServer complete the stock-compatible security/stream setup and decode the first client packet. | runtime initialization | Role entry and every client-facing feature | socket stream/acceptor, protocol/transport dossier, L4/L5 | card-open |
| `FND-RELAY-ROLE-ENVELOPE` | GameServer and Center preserve the required role-section and Relay envelope contracts on the active path. | runtime initialization | Role load, save/relogin, cross-GS features | `KRelayClient`, `KPlayerServer`, role types, L5 | card-open |
| `FND-ROLE-LOAD-SPINE` | A role is loaded with the required versioned base/state sections and without shifting downstream role data. | relay role envelope | Enter world and persistence features | `KPlayer::LoadStateInfo_*`, role section sync, DWARF layouts | card-open |
| `PLY-ENTER-WORLD` | Select a role and enter a scene with ready confirmation and initial player/NPC/doodad synchronization. | secure transport, relay role envelope, role load spine | Movement, NPC interaction, combat | `KPlayerServer::OnApplyCharacter`, `OnClientConfirmReady`, `OnSyncNew*Respond`; protocol/type dossiers | card-open; implementation blocked by foundation |

### T2 — Basic world interaction

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `PLY-MOVE-AND-POSITION-SYNC` | A player moves and receives authoritative position/state synchronization, including the selected sprint/jump behavior. | enter world | Exploration and combat positioning | `KGMoveProcessor`, `KGJumpList`, `SYNC_MOVE_*`, `KCharacter::Sprint*`, `KPlayerServer` motion handlers; L4/L5 | blocked-by-foundation |
| `NPC-SPAWN-AND-STANDARD-AI` | A normal NPC is created from target data and completes standard AI setup without Lua/enum/binding failure. | enter world, runtime initialization | Combat, NPC drops, quest interactions | `KAIManager`, `KAILogic`/`KAIVM`/`KAIAction`/`KAIState`, `NpcTemplate.AIType`, `AIType.tab`, `StandardAI.lua`; L2/L4/L6 | blocked-by-foundation |
| `WORLD-NPC-REVIVE-AND-DOODAD` | Selected NPC/doodad lifecycle and respawn behavior work in a bounded scene. | NPC standard AI | Gathering and selected activities | `KNpcReviveManager`, `KDoodadReviveManager`, `KScene`, target table/script readers | catalogued |

### T3 — Minimum combat loop

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `COMBAT-CAST-ONE-SKILL` | A player casts one selected skill on an NPC and target state changes through the target-backed skill/buff path. | movement, NPC standard AI | Death, threat, progression | `KSkill`, `KBuffList`, `KSkillManager`, `KCharacter`, skill Lua and enums; L2/L4/L7 | blocked-by-foundation |
| `COMBAT-THREAT-AND-DEATH` | The selected NPC reacts, takes damage, reaches death, and executes the required scene cleanup path. | cast one skill, NPC standard AI | Loot and experience | `KCharacter` threat methods, `KSimpThreatList`, `KNpc`, `KScene` | blocked-by-foundation |
| `LOOT-NPC-TO-INVENTORY` | A selected NPC death grants the intended loot/experience result into the player's inventory/state. | combat threat and death | Economy and persistence | `KDropCenter`, `KManualDropList`, `KLootList`, `KNpc`, selected `KItemList` methods, drop tables; L3/L5 | blocked-by-foundation |

### T4 — Persistence and core progression

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `PLY-SAVE-LOGOUT-RELOGIN` | The selected role state survives save, logout, and relogin without blob/version/section corruption. | enter world, relay role envelope | Every persistence-heavy feature | `KPlayer::Save*`/`Load*`, `KItemList` versioned load/save, Relay role sections, L5 | blocked-by-foundation |
| `ITEM-EQUIP-AND-USE` | A normal item can be acquired, equipped or used, and applies the intended player-visible state. | loot to inventory, save/relogin | Equipment and economy features | selected `KItemList` equip/use/require methods, item tables, Lua bindings | blocked-by-foundation |
| `PROGRESSION-LEARN-AND-USE-SKILL` | A selected skill can be learned or changed, saved, and used in the basic combat path. | cast one skill, save/relogin | Talent, recipe, school progression | `KSkillList`, `KSkillManager`, `KSkillRecipeList`, player skill persistence | catalogued |
| `QUEST-ONE-LIFECYCLE` | One quest can be accepted, completed, rewarded, and preserved across the selected role lifecycle. | item use, save/relogin | Wider quest/content work | `KQuestList`, `KQuestInfoList`, quest tables/scripts, player quest block | catalogued |

### T5 — Economy, appearance, and extended progression

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `ECONOMY-BASIC-CURRENCY-PURCHASE` | A bounded currency/shop purchase changes currency and grants the intended item/state. | inventory, save/relogin, relay role envelope | Shop variants and crafting | `KCurrency*`, `KShop*` or `KRewardsShop*`, matching Relay route, shop tables | catalogued |
| `COSMETIC-EXTERIOR-JOURNEY` | A selected exterior purchase/apply/persist path works. | inventory, currency where applicable, save/relogin | Wider appearance features | `KExterior*`, player exterior sync, exterior tables/Lua | historical-revalidation |
| `COSMETIC-HAIR-JOURNEY` | A selected hair purchase/change/persist path works. | inventory, currency where applicable, save/relogin | Wider appearance features | `KHairShop`, `KHairBox`, matching Relay route and tables | historical-revalidation |
| `COSMETIC-TITLE-AND-MINIAVATAR` | A selected title or mini-avatar acquire/equip/sync path works. | save/relogin, role sync | Cosmetic completion | `KDesignation`, `KMiniAvatar*`, `KNewExtPointManager` | historical-revalidation |
| `PET-ACQUIRE-AND-EVOKE` | A selected pet can be acquired, persisted, and evoked into the scene. | inventory, save/relogin, NPC scene | Pet growth and related content | `KDomesticate*`, `KFellowPet*`, player pet import/export, Lua/table evidence | catalogued |
| `PROGRESSION-TALENT-SET` | A selected talent set can be opened, updated, saved, and applied to skill behavior. | learn/use skill, save/relogin | Extended class progression | `KTalentSkill`, player talent Lua methods, skill/talent data | catalogued |
| `PROGRESSION-REPUTATION-ACHIEVEMENT` | Selected reputation and achievement updates are granted and persisted. | combat/loot or quest, save/relogin | Extended progression | `KReputation`, `KAchievement*`, loot/quest triggers | catalogued |
| `ACTIVITY-AND-CAMP-STATE` | A selected activity/camp event starts, changes state, and reaches its scene/player effect. | enter world, NPC scene, relay where used | Seasonal/activity content | `KActivityMgrServer`, `KCampInfo`, `KCampActiveStat`, Lua triggers/tables | catalogued |

### T6 — Cross-process, social, and operational content

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `WORLD-MAP-AND-COPY-ROUTE` | A selected map/copy transition preserves role state across the required process and scene boundaries. | enter world, save/relogin, relay envelope | Dungeon and cross-GS content | `KScene`, `KSO3World`, map data, Relay route/type dossiers | catalogued |
| `SOCIAL-GUILD-JOURNEY` | A selected guild membership/cache/sync journey works through Center and player state. | relay role envelope, save/relogin | Guild content | `KTongServer`, `KTongDiplomacyCache`, corresponding `KRelayClient` handlers, Lua | catalogued |
| `SOCIAL-MENTOR-JOURNEY` | A selected mentor relationship is created/synced/persisted. | relay role envelope, save/relogin | Mentor rewards/content | `KDirectMentorCache`, `KMentorCache`, Relay and player mentor block | catalogued |
| `SOCIAL-PK-OR-ARENA-JOURNEY` | One selected duel, PK, or arena queue route completes with its required Relay and player transitions. | combat, relay role envelope | Broader PvP | `KPKController` or arena handlers in `KPlayerServer`/`KRelayClient` | catalogued |
| `SOCIAL-RANKING-SYNC` | A selected ranking update/query route synchronizes correctly. | relevant score-producing feature, relay | Score/rank content | `KRankListServer`, single-dungeon score handlers, Relay | catalogued |
| `OPERATIONS-ANTI-FARMER` | Selected anti-farmer policy/state updates do not break normal player entry or monitoring behavior. | enter world, relay where used | Operational completeness | `KAntiFarmer*`, bandwidth tactics, config data | catalogued |
| `OPERATIONS-GM-AND-RECORDER` | GM permission tables and selected recorder/replay cleanup behavior are target-compatible. | runtime initialization | Operations tooling | `KGMList`, `KRecorderSceneLoader*`, config data | catalogued |

### T7 — Hardening and residual closure

| ID | Capability | Depends on | Unlocks | Evidence leads | Status |
|---|---|---|---|---|---|
| `HARDEN-RECONNECT-AND-REPEAT-PERSISTENCE` | Repeated disconnect/reconnect and persistence cycles preserve the accepted core state. | save/logout/relogin and active features | Release confidence | paired runtime traces, role/blob diffs, process provenance | blocked-by-foundation |
| `HARDEN-MALFORMED-INPUT-AND-LUA-FAILURE` | Bounded malformed input and Lua error paths fail safely without corrupting server/player state. | secure transport, active Lua features | Release confidence | protocol boundary captures, Lua error policy, logs | blocked-by-foundation |
| `HARDEN-COMPLETENESS-AUDIT` | Deferred catalogue items are classified as accepted, explicitly deferred, or blocked with evidence. | selected feature catalogue | Final scope statement | L1–L7 refresh, GraphEngine coverage, Feature Card index | blocked-by-foundation |

## From timeline entry to Feature Card

Before creating a card, the orchestrator must confirm all of the following:

1. The entry has one observable outcome and a bounded state transition.
2. Prerequisite entries are accepted cards, or the card explicitly limits itself
   to static evidence and names the blocker.
3. Target artifact identities and first evidence roots can be recorded.
4. The needed L1–L7/GraphEngine checks are selected by boundary, not run as a
   blanket ritual.
5. A stock/candidate harness can be described for behavior claims. A
   static-only card must say why it makes no runtime claim.
6. The implementation closure has named ownership and does not silently claim
   an entire hub such as `KPlayer`, `KRelayClient`, or `KScriptFuncList`.

Only then copy [port-card.tmpl](port-card.tmpl) into a Feature Card. Its status
starts at `draft` or `researching`; historical notes never start it at
`accepted`.

## From Feature Cards to a Wave

A Wave is created just in time when a small contiguous part of this roadmap
has a stable integrated scenario. It contains only the cards required to prove
that scenario. The Wave is accepted only when every required card is accepted
and the integrated scenario passes; the timeline itself never closes a Wave.

For example, the first player-visible Wave can be formed only after the
foundation and `PLY-ENTER-WORLD` entries have become real cards with defined
contracts. It must not be created from a generic label such as “login work” or
from a list of source files.
