# JX3 Server Port Plan: 2010 Source to v2.5.2

This directory is the public planning and evidence map for reconciling the 2010
GameServer source with the v2.5.2 target server. Work is organised around
**player-visible features**, not a queue of files or a list of symbols to copy.

The purpose is to make each port decision auditable:

1. establish the target contract;
2. map the 2010 baseline and current candidate implementation;
3. bound the dependencies required by one feature;
4. implement the smallest coherent change; and
5. verify it with target or runtime evidence.

This is not a claim that the complete server has been ported.

## Authority and evidence

The v2.5.2 target defines the target contract. The 2010 source is the
implementation baseline; it cannot override a target binary or DWARF fact.

| Question | Primary authority | Supporting evidence |
|---|---|---|
| Target function/type/address | Target DWARF and binary | target decompile |
| Class hierarchy, field offset/type/size | Target DWARF | target decompile |
| Control flow and call relationships | Target decompile/disassembly | runtime trace |
| Protocol route and payload contract | Target binary plus packet/runtime evidence | DWARF layout |
| Lua registration and object shape | Target decompile/binary | deployed scripts |
| `.tab`/`.ini` reader semantics | Target decompile plus table header/data | boot/runtime observation |
| Original implementation logic | 2010 source | source history |
| Candidate behavior | Candidate source and rebuilt artifact | build provenance and runtime capture |

An unresolved fact is a valid result. It must not be filled in by assuming that
a similarly named 2010 method describes the target.

## Planning model

```text
Wave
  = release and dependency container

Feature Slice
  = one player-visible capability with a bounded evidence closure,
    implementation scope, review, and acceptance scenario

Methods / types / enums / packets / Lua / tables / role blocks
  = evidence needed by the Feature Slice, not independent tickets by default
```

Create a [Feature Card](port-card.tmpl) before editing source. It must state the
target artifact, source/candidate mapping, required contracts, explicit
deferrals, and a bounded runtime acceptance scenario. A successful build is not
acceptance by itself.

Use [FEATURE_ROADMAP.md](FEATURE_ROADMAP.md) to select the next capability and
to determine when it is ready to become a Feature Card or a Wave. The roadmap
does not grant acceptance status.

## Directory map

| Path | Purpose |
|---|---|
| `PORT_MAP.csv` | Generated machine-readable method inventory; do not hand-edit. |
| `FEATURE_ROADMAP.md` | Capability dependency graph and protocol for creating Feature Cards and Waves. |
| `WAVES.md` | Wave-level dependency scope and acceptance milestones. |
| `LANE_B_LEAF_PORTS.md` | Parallel leaf-subsystem port lane: work set, execution model, and acceptance bar that needs no client. |
| `waves/` | The one canonical home for every opened Wave: `WAVE.md`, `STATE.md`, Wave-owned Feature Cards, and local `evidence/`; `W1-FOUNDATIONS-AND-ENTRY` is active, not accepted. |
| `layers/` | Inventory plus L1-L7 producer-and-evidence layers. |
| `knowledge_base/` | Static surveys of code systems, data tables, and Lua systems. |
| `port-card.tmpl` | Required contract template for a Feature Slice. |

Read [layers/README.md](layers/README.md) for the exact questions, authority,
outputs, and blind spots of the inventory and L1-L7 layers. The Knowledge Base
is context; it does not override target evidence.

## Method inventory

`PORT_MAP.csv` is an investigation index. It is not a top-to-bottom implementation
queue and it does not determine feature priority.

The generation process:

1. extracts class-scoped functions emitted in target `.text`;
2. extracts the candidate symbol surface;
3. computes target-minus-candidate symbols;
4. checks the 2010 source to reduce false missing symbols; and
5. records class, method, signature, cluster, NEW/DRIFT type, and confidence.

| Classification | Meaning | Correct response |
|---|---|---|
| `PRESENT_EXACT` | The same class/method is visible in source; it may be inline or not emitted. | Do not treat it as missing without semantic evidence. |
| `NAME_ELSEWHERE` | The bare method name occurs in another class. | Investigate rename/relocation; confidence is low. |
| `TRULY_MISSING` | No matching name is visible in the source baseline. | Research target behavior before implementing. |
| `NEW` class | Target class has no candidate symbol surface. | Treat it as a subsystem candidate, not automatic priority. |
| `DRIFT` class | The class exists on both sides but method surface differs. | Select only methods on an active feature path. |

The current inventory (regenerated 2026-07-28) reports 4,820 target class-scoped
methods across 235 classes, 3,485 candidate methods across 207 classes, and 1,458
rows marked `TRULY_MISSING` plus 139 `NAME_ELSEWHERE`. Of the 1,597 rows, 1,260 are
`DRIFT` and 337 are `NEW`. The previous 2026-07-11 snapshot reported 3,335 candidate
methods and 1,716 rows; 121 rows have closed since. These are inventory statistics,
not completion percentages: inline/template code and compiler emission can change
the count, and symbol existence does not prove correct logic — a method can be
present, compile, and still carry the drift that breaks the feature. Exact inputs,
identities, and the regeneration record are in
[layers/inventory](layers/inventory/) and
[layers/inventory/evidence/REGENERATION.md](layers/inventory/evidence/REGENERATION.md).

## Feature dependency structure

The main gameplay hubs are player core, skill/Lua, world/scene/AI, and item
equipment. Combat is not a leaf class: it emerges from character state and
damage, skill/buff execution, scene/NPC/AI behavior, and item attributes.

```text
player-core       => skill-script, item/equipment, world-scene, movement
skill-script      => player-core, item/equipment
world-scene       => player-core, skill-script, movement
item/equipment    => player-core
drop-loot         => item/equipment, world-scene, player-core
economy/cosmetic  => item/equipment, currency, player-core
quest/social/pet/activity/progression => player-core
network-infra     = transport and cross-process contracts for every domain
```

`=>` means a hard dependency on the active feature path. This graph helps scope
a Feature Card; it does not require porting a whole hub in one change.

## Serialization spine

Most methods in a large hub can be ported with the feature that needs them. The
serialization spine is the exception and must be reconciled coherently wherever
the feature crosses it.

The spine includes:

1. `KPlayer` save/load sub-blobs and versioned decoders;
2. role-base and role-section synchronization with GameCenter;
3. packet headers, packing, field order, and byte size; and
4. any persisted or cross-process structure used by the feature.

A one-byte mismatch can invalidate an otherwise correct feature. L5 provides
size evidence, but a matching size is not enough: field order, conditional
encoding, and semantics still require target evidence.

## Feature domains and milestones

Priority comes from an active player scenario plus target evidence, never from
a raw count of missing methods.

| Domain | Role | Typical dependencies |
|---|---|---|
| Player core | role lifecycle, login, save/load | nearly every system |
| Skill, buff, Lua | casting and effect execution | player, item, world |
| World, scene, NPC, AI | spawn and gameplay state | player, skill, movement |
| Item/equipment | inventory and attributes | player, skill, economy |
| Movement and drop/loot | core-loop transitions | player, world, packets |
| Quest and progression | schools, talent, achievement, reputation | player, skill, item |
| Economy and cosmetic | shop, currency, appearance | player, item, persistence |
| Social, pet, activity | guild, mentor, camp, content subsystems | player, GameCenter, Lua |
| Network infrastructure | transport, relay, security, RPC | all process boundaries |

| Milestone | Player outcome | Minimum acceptance direction |
|---|---|---|
| M0 — transport/config readiness | Client reaches the GameServer and the first protocol boundary is proven. | Paired target/candidate endpoint, security/handshake, and first decoded-packet evidence. TCP accept alone is insufficient. |
| M1 — core loop | Enter world, move, fight an NPC, receive loot/experience, and persist a role. | Bounded end-to-end scenario with AI, skill, player spine, packet, and data contracts. |
| M2 — progression | Quest, school/skill progression, talent, achievement/reputation, and a bounded currency loop. | Separate Feature Cards for each player journey. |
| M3 — social | Selected guild, mentor, camp/activity, PK, or rank functionality. | Explicit cross-process and persistence checks. |
| M4 — QoL/content | Cosmetic, pet, operational, and remaining non-core content. | Feature-by-feature target contract and runtime acceptance. |

Historical failures and prior wave conclusions are useful clues only. They do
not replace a current target dossier, candidate build provenance, or runtime
scenario.

## Standard feature workflow

1. Choose one player outcome, such as enter scene, cast one skill, loot one NPC,
   buy one item, or persist one role state.
2. Create a Feature Card with scope, exclusions, target artifact, acceptance
   scenario, and explicit unknowns.
3. Build the required evidence closure from the inventory, L1-L7, DWARF, and
   decompile; use only layers that touch the feature path.
4. Define invariants: protocol/layout, save/load, Lua registration, table reader,
   DSO/ABI, and runtime state boundaries as applicable.
5. Implement narrowly. Do not combine unrelated refactors or broad file ports.
6. Cross-review the evidence and diff with a different model/operator.
7. Build with recorded candidate artifact provenance.
8. Run the bounded target/candidate oracle. State unresolved facts explicitly.
9. Update the Feature Card and the relevant layer evidence; do not inflate a
   general completion metric from one accepted slice.

## Non-goals and guardrails

- A clean boot log is not gameplay acceptance.
- Symbol coverage is not semantic equivalence.
- A matching protocol ID is not a matching payload contract.
- Source 2010 cannot substitute for target v2.5.2 evidence.
- A class inventory gap is not permission to port every method in that class.
- Public documentation must not contain private deployment details, account data,
  database dumps, raw runtime logs, or large private reverse-engineering corpora.

The plan is intentionally compact: it should be enough to select and verify a
feature while keeping raw target artifacts, GraphEngine data, and private runtime
captures in their appropriate locations.
