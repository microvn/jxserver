---
feature_id: "FND-RUNTIME-INITIALIZATION"
title: "Target-bounded GameServer runtime initialization"
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "reviewing"
priority: "P0"
kind: "foundation"
owner: "orchestrator"
producer_model: "pending"
reviewer_model: "pending"
planned_producer_model: "codex"
planned_reviewer_model: "claude"
graph_db_sha256: "25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba"
target_artifacts:
  - "SO3GameServerD sha256:47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a"
  - "SO3GameServer sha256:3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11"
evidence_package: "linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/EVIDENCE_RUNTIME_INIT.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/CANDIDATE_P3_PROVENANCE.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/EXACT_STOCK_RUNTIME_BASELINE_2026-07-27.md"
harness_contract: "pending"
evidence_state: "runtime-observed — exact target stock completes boot and user-observed enter-world; no candidate comparison is accepted"
runtime_constraint: "candidate source/build provenance and paired runtime arm remain required"
candidate_baseline: "git 4e60f1697bf71f2dd46ff390f24f3a6a357e300f; manifest must be created before an edit"
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-27T02:22:01Z"
next_action: "Use the exact-stock runtime arm as the only oracle; re-run a hash-pinned candidate under identical topology before interpreting any entry failure."
---

# Feature Card: FND-RUNTIME-INITIALIZATION — GameServer runtime initialization

## 1. Outcome

### Capability

The rebuilt GameServer initializes the required data, scripts, DSOs, locale,
and map/config inputs to a target-bounded boot fingerprint.

### Observable result

- [ ] Candidate reaches each named boot milestone selected from the stock arm.
- [ ] Candidate retains only the stock-compatible known non-fatal fingerprint;
      it does not hide a missing loader contract by filtering errors.

### Explicit non-goals

- Client login, role selection, or scene synchronization.
- Claiming every stock log line is reproduced or every target data asset is
  understood.

## 2. Context and dependencies

### Why this slice now

All subsequent runtime features consume loader output. A boot that only looks
healthy can still omit AI, Lua, tables, DSO registration, or maps; the card
therefore owns a bounded initialization contract before entry work.

### Prerequisite Feature Cards

| Feature ID | Required result | Status at start |
|---|---|---|
| — | Target boot artifacts and selected target data are available. | available, unpinned for this card |

### State-machine boundary

```text
process not started → configured GameServer start → required initialization milestones reached
```

### Consumers and downstream features

| Consumer / next feature | Relation | Impact if wrong |
|---|---|---|
| all W1 cards | consumes initialized managers/data/DSOs | later failures become misleading loader fallout |
| `NPC-SPAWN-AND-STANDARD-AI` | consumes AI/table/Lua startup results | NPC behavior cannot be trusted |

## 3. Target contract (v2.5 authority)

### Target artifacts

| Artifact | SHA-256 | Authority for | Notes |
|---|---|---|---|
| `SO3GameServerD` | `47716c73…51124b94a` | named initialization layout/symbol roots | DWARF target |
| `SO3GameServer` | `3002bf4a…5c416f11` | stock runtime fingerprint | stripped runtime target |

### Target roots

| Domain | Exact identity | Required fact | Evidence ID / raw artifact |
|---|---|---|---|
| Function | `KSO3World`, `KWorldSettings`, `KNpcTemplateList` | initialization order and failure consumers | pending dossier |
| Table | target loader inputs | reader/header/column use on selected boot path | pending L3 evidence |
| Lua | initialization registrations/scripts | required binding/load shape | pending L2/L6 evidence |
| Transport / DSO | linked provider/register path | DSO/provider identity | pending build/decompile evidence |

### Target behaviour / invariants

1. Target boot milestones and the selected known-failure fingerprint must be
   measured from a fresh stock arm before candidate comparison.
2. Every claimed candidate improvement must map to a named target consumer;
   error suppression is not an implementation.

## 4. Origin comparison and Graph dossier

### Required read-only dossiers

```text
compare-engine/tools/query/evidence_query.py function KSO3World::Init
compare-engine/tools/query/evidence_query.py function KWorldSettings::Load
After the boot closure identifies one table reader, record and run the exact
`evidence_query.py tab` command here. No target table is asserted yet.
```

### Evidence classification

| Class | Claim / observation | Raw evidence location | Consumer / decision |
|---|---|---|---|
| Confirmed fact | Target binary identities are pinned above. | artifact hashes | research scope only |
| Inference | Historical loader fixes may be relevant. | historical states only | revalidate; never accept directly |
| Unresolved / conflict | target order, selected counts, DSO/provider and error classes | pending saved dossier/capture | blocks contract-ready |

### Unknown policy

| Unknown / conflict | On required path? | Resolution evidence needed | Decision |
|---|---:|---|---|
| Which loader errors are stock-valid | yes | fresh stock fingerprint | block implementation claim |
| DSO/provider ownership | yes | target import/decompile plus build manifest | block provider change |

## 5. Boundary matrix

| Boundary | Required contract | Candidate implementation point | Verification |
|---|---|---|---|
| C++ logic | initialization order | pending target closure | decompile comparison |
| Lua | required startup loads | pending | stock/candidate load observation |
| Table/config | selected reader/header semantics | pending | L3 + target input hash |
| Build/DSO | provider/link/runtime input identity | pending | build-input manifest |

All other boundaries are `N/A` until the selected initialization closure proves
they are consumed.

## 6. Candidate implementation scope

No write lease exists. Source paths, allowed writes, overlay, and backup root
are intentionally pending the target closure. Forbidden now: loader-error
suppression, bulk table edits, DSO substitution, and unrelated refactoring.

## 7. Acceptance checklist

- [ ] Target roots, stock boot arm, and selected target data/config identities recorded.
- [ ] Evidence package and opposite-model research review recorded.
- [ ] Baseline/overlay/build provenance gate passes before any source edit.
- [ ] Fresh native-x86 build and active executable provenance recorded.
- [ ] Paired boot contract passes without hiding semantic errors.
- [ ] Rollback artifact retained.

## 8. Must-stay-unchanged invariants

| Invariant | Baseline evidence | Candidate result | Verdict |
|---|---|---|---|
| Stock executable and target data remain untouched. | target hashes above | pending | pending |

## 9. Deferrals, risks and handoff

`AITypeList`/standard AI, later Lua gameplay, and scene entry are deferred to
their owning cards unless the target boot closure proves a direct prerequisite.
The handoff must add graph DB hash, raw evidence, review verdict, build/runtime
bundle, rollback identity, and one executable next action.
