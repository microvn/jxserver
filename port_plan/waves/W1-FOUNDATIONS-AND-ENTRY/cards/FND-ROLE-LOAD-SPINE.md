---
feature_id: "FND-ROLE-LOAD-SPINE"
title: "Versioned role-load spine"
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "researching"
priority: "P0"
kind: "foundation"
owner: "orchestrator"
producer_model: "pending; no source-write lease"
reviewer_model: "pending; review begins only with an atomic raw diff"
planned_producer_model: "codex"
planned_reviewer_model: "claude"
graph_db_sha256: "25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba"
target_artifacts:
  - "SO3GameServerD sha256:47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a"
evidence_package: "linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/role-load/ROLE_LOAD_DOSSIER.md"
harness_contract: "static version/layout proof, then paired role-envelope capture and bounded role-block round trip"
evidence_state: "static-confirmed — target version dispatcher/base layout and block namespace are pinned; delivered block version/round trip are unmeasured"
runtime_constraint: "Windows client unavailable; role-envelope block headers and paired role-load observation remain runtime-pending"
candidate_baseline: "git 4e60f1697bf71f2dd46ff390f24f3a6a357e300f; manifest required before edit"
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-26T18:20:24Z"
next_action: "Use the pinned Center peer to complete static producer/consumer closure; defer role-envelope block-header capture until a selected-role client transition is available."
---

# Feature Card: FND-ROLE-LOAD-SPINE — versioned role loading

## 1. Outcome

### Capability

A selected role is loaded through the required versioned base/state sections
without shifting downstream role data on the entry path.

### Observable result

- [ ] The selected role reaches the named target-backed post-load state.
- [ ] Required base/state sections retain their target-backed layout/order;
      unknown or later sections follow an explicit preserve/defer policy.

### Explicit non-goals

- Full save/logout/relogin acceptance.
- Every historical account/role blob format or all optional player subsystems.

## 2. Context and dependencies

### Why this slice now

Role loading is a shared serialization spine. A single inserted field or wrong
section boundary can allow character listing but corrupt map/state consumers
later. The card therefore precedes scene entry and broader persistence.

### Prerequisite Feature Cards

| Feature ID | Required result | Status at start |
|---|---|---|
| `FND-RELAY-ROLE-ENVELOPE` | target-backed envelope/section delivery | researching |

### State-machine boundary

```text
role envelope complete → base/state sections decoded/loaded → post-load role state available to scene entry
```

### Consumers and downstream features

| Consumer / next feature | Relation | Impact if wrong |
|---|---|---|
| `PLY-ENTER-WORLD` | reads loaded map/position/state | entry refusal or wrong scene |
| `PLY-SAVE-LOGOUT-RELOGIN` | reuses versioned data | later blob corruption |
| inventory/skill/quest features | consume distinct player blocks | latent shifts/incorrect defaults |

## 3. Target contract (v2.5 authority)

### Target artifacts

| Artifact | SHA-256 | Authority for | Notes |
|---|---|---|---|
| `SO3GameServerD` | `47716c73…51124b94a` | role type fields, sizes, load symbols | target DWARF |
| `SO3GameServer` | `3002bf4a…5c416f11` | target runtime arm | stripped stock |

### Target roots

| Domain | Exact identity | Required fact | Evidence ID / raw artifact |
|---|---|---|---|
| Function | `KPlayer::LoadStateInfo` at `0x0839b112`; `KPlayer::LoadBaseInfo` at `0x0839dce2` | version dispatch and map-entry base load | role-load dossier |
| Type / field | `KRoleBaseInfo` DWARF DIE `0x00a7fdfd`, size `0xa2`; target role blocks through `rbtTotal=49` | size/offset/type and namespace | role-load dossier |
| Save/load | selected role blocks | versions, boundaries, opaque policy | target parser + paired capture still required |
| Protocol | envelope-to-load handoff | payload ownership and state transition | pending Relay link |

### Target behaviour / invariants

1. Source-2010 field order never overrides target DWARF offsets.
2. The role's map/position and completion flags are accepted only when their
   target consumers and serialized boundaries are proven.

## 4. Origin comparison and Graph dossier

### Required read-only dossiers

```text
graphengine/tools/query/evidence_query.py function KPlayer::LoadStateInfo
graphengine/tools/query/evidence_query.py type KROLE_POSITION_DB
After the target load closure identifies the first post-load consumer, record
the exact `graph_query.py callers` command.
```

### Evidence classification

| Class | Claim / observation | Raw evidence location | Consumer / decision |
|---|---|---|---|
| Confirmed fact | target GS artifact identity is pinned | artifact hash | scope anchor |
| Inference | historical role layout changes may be relevant | historical states | revalidate against DWARF |
| Unresolved / conflict | exact load closure, sections, versions, offsets, round-trip | pending dossier/capture | blocks implementation |

### Unknown policy

| Unknown / conflict | On required path? | Resolution evidence needed | Decision |
|---|---:|---|---|
| role field offset/size/type | yes | DWARF | block |
| section version/boundary/consumer | yes | target decompile + paired observation | block |

## 5. Boundary matrix

| Boundary | Required contract | Candidate implementation point | Verification |
|---|---|---|---|
| C++ logic | load sequence and state transition | pending | target decompile |
| ABI/layout | selected role structs | pending | DWARF/assert/mirror |
| Protocol/transport | Relay-to-load handoff | pending | paired capture |
| Persistence | section/version/round-trip | pending | blob/DB-state comparison |
| Build/DSO | relevant serializers/providers | pending | build manifest |

Lua and table/config are `N/A` unless the first post-load closure proves a
direct dependency.

## 6. Candidate implementation scope

No write lease exists. Forbidden: adding/removing/reordering serialized fields
from source intuition, broad player-class copy, destructive migration of role
data, or accepting an unproven blob by truncation.

## 7. Acceptance checklist

- [ ] Target load functions/type layouts and first map-entry consumer are recorded; role-block version delivery remains open.
- [x] Graph/dossier preserves source/candidate/target observations separately and retains unresolved coverage explicitly.
- [ ] Paired role-load/round-trip observation meets named invariants.
- [ ] Opposite-model review, baseline manifest, clean native build, and process hash pass.
- [ ] Required unknown section policy is measured.
- [ ] Rollback artifact remains available.

## 8. Must-stay-unchanged invariants

| Invariant | Baseline evidence | Candidate result | Verdict |
|---|---|---|---|
| Unowned player blocks and role blobs are not rewritten. | pre-run hash/DB evidence pending | pending | pending |

## 9. Deferrals, risks and handoff

Item, skill, quest, and save/relogin semantics are deferred to their cards.
Handoff preserves raw layout/decompile/capture evidence, explicit unknown
blocks, source baseline/overlay hashes, review, and rollback data.
