# JX3 Port Waves

This document defines the dependency order and acceptance boundary of the JX3
server port. A Wave is a release/dependency container, not a file queue. Work
inside a Wave is performed as Feature Cards using `port-card.tmpl`.

`FEATURE_ROADMAP.md` is the canonical capability/dependency catalogue and
`waves/<WAVE-ID>/WAVE.md` is the canonical record for an opened Wave. The
numbered map below is a legacy coarse dependency sketch, not a list of opened
Wave IDs. In particular, the first opened Wave is
[`waves/W1-FOUNDATIONS-AND-ENTRY/WAVE.md`](waves/W1-FOUNDATIONS-AND-ENTRY/WAVE.md)
and is currently **planned**, not accepted.

## Global gates for every Wave

Every Wave starts from a canonical accepted baseline and ends only at the
furthest milestone proven by evidence.

1. Layout/ABI claims have target DWARF/decompile evidence before logic changes.
2. Candidate source, build inputs, and active binary have recorded provenance.
3. GraphEngine evidence is used by origin and status; static evidence is not
   treated as runtime parity.
4. Stock and rebuilt behavior are compared for every claimed runtime scenario.
5. Unknowns on a required protocol, persistence, or consumer path are blockers.
6. A Feature Slice is accepted only with its explicit evidence closure, build
   gate, review, and bounded runtime oracle.

## Dependency spine

```text
artifact identity + ABI/client/relay/persistence contracts + stock scenario harness
  -> fresh build and cold boot
  -> GS-Center handshake, client wire, role preservation
  -> login/session/load
  -> enter scene and object synchronization
  -> movement, AI, inventory
  -> skill/combat/death
  -> loot/experience/currency
  -> save/logout/relogin
  -> map/cross-GS, global/social, remaining features
  -> soak, reconnect, completeness hardening
```

The serialization spine is special: role blobs, GameCenter section sync, packet
headers/packing, and every persisted/cross-process type must be reconciled
coherently on the active feature path. Other hub handlers may remain scoped to
their individual Feature Slice.

## Wave map

| Wave | Scope | Entry condition | Done condition |
|---|---|---|---|
| 0 — Pilot | Validate agent factory, isolated worktrees, fail-fast build, and a small ABI manifest. No gameplay port. | Clean committed baseline and leases. | Fresh build cannot publish stale output; independent verifier reproduces selected DWARF contracts. |
| 1 — Runtime shell and cold boot | Data loaders, DSO integration, locale/config/map paths, stock error fingerprint, rebuilt initialization. | Build provenance and target config/loader evidence. | Rebuilt server completes bounded initialization with stock-compatible fingerprint; no unproven loader contract is hidden. |
| 2 — Contract foundation | Client wire, Relay wire, persistence envelope, opaque role-block preservation. | Target enum/layout/route evidence. | Required assertions pass; stock Center remains connected; required role blocks round-trip byte-for-byte. |
| 3 — Login and enter scene | Session attach/load, client login, scene entry, ready, initial object sync, cleanup/reconnect. | Waves 1–2 contracts proven for this path. | Character selection enters map, ready/object sync succeeds, and bounded logout/reconnect works. |
| 4 — Movement, AI, item core | Movement/jump correction, NPC/AI/threat, inventory/equip/use and required Lua/table contracts. | Enter-scene state is stable. | Selected movement, NPC reaction, and inventory scenarios pass with target contract evidence. |
| 5 — Combat, loot, core persistence | Skill/buff, damage/threat/death, loot/drop/experience/currency, save/relogin. | Wave 4 dependencies pass for the selected loop. | A bounded core loop is playable and survives save/logout/relogin. |
| 6 — Cross-GS and global/social | Map switch, dynamic copies, cross-GS, team, mail, guild, mentor, auction/global values. | Role and Relay contracts proven per hop. | Selected cross-process route has target-addressed protocol/type evidence and runtime acceptance. |
| 7 — Remaining v2.5 subsystems | Feature-driven vertical slices across movement/world, item/drop/pet/progression, activity/social/global, and Lua glue. | Relevant earlier contracts accepted. | Each selected subsystem reaches its own Feature Card acceptance; no bulk class-copy campaign. |
| 8 — Hardening | Reconnect, repeat persistence, malformed packet, Lua error, performance/tick soak, reachability/completeness audit. | Core and selected extended features accepted. | Stock/rebuilt differential scenarios have no unexplained divergence on the stated scope. |

## Required GraphEngine use by Wave

GraphEngine is a research and evidence gate. It does not close a Wave on its
own. Use the relevant dossier family before each Feature Slice:

| Boundary | Required evidence direction |
|---|---|
| Loader/DSO/config | Function dossier and target caller/callee closure; table evidence where used. |
| Protocol/Relay | Protocol dossier, handler closure, payload/type evidence, target binary identity. |
| Role persistence | Type dossier, field layout, save/load consumer evidence, runtime round-trip. |
| Lua feature | Registration and object-shape evidence, target call path, script/table context. |
| Gameplay handler | Function dossier, callers/callees, dependent types/protocols, bounded runtime scenario. |

Record every `partial` or `UNRESOLVED` result in the Feature Card. It is accepted
only when it is outside the selected feature path; otherwise it blocks the
claim, not necessarily all research.

## Scope discipline

- Do not declare an entire Wave complete because a server process starts.
- Do not re-open a completed lower Wave without recording the earlier divergence
  and exact evidence that invalidated it.
- Do not port all target-only compilation units simultaneously. Use vertical
  slices: data -> manager -> player state -> Lua -> protocol -> persistence ->
  relogin where applicable.
- Do not use a generic client error string as root cause; locate the first
  divergent state transition, packet boundary, ABI read, or persistence block.
- Historical logs and prior wave reports are clues, not current oracle evidence.

## Updating this plan

Update a Wave only when its dependency boundary, active Feature Slices, or
proven acceptance condition changes. Put implementation-level evidence in the
Feature Card and GraphEngine; keep this file at Wave granularity.
