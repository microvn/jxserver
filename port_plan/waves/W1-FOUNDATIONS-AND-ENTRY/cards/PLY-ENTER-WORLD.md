---
feature_id: "PLY-ENTER-WORLD"
title: "Select a role and reach scene-ready"
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "live-verifying"
priority: "P0"
kind: "integration"
owner: "orchestrator"
producer_model: "codex — B1 account Relay/parser isolated overlay"
reviewer_model: "claude — R1 opposite-family review PASS WITH CAVEATS (static only)"
planned_producer_model: "codex"
planned_reviewer_model: "claude"
graph_db_sha256: "25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba"
target_artifacts:
  - "SO3GameServerD sha256:47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a"
  - "SO3GameServer sha256:3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11"
evidence_package: "linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/W1_STATIC_CLOSURE_MATRIX.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/entry/raw/w1-entry-data-target.jsonl; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/entry/ENTRY_STATIC_DOSSIER.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/entry/ACCOUNT_EXT_DATA_STATIC_DOSSIER.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/entry/S2C_SYNC_PLAYER_BASE_INFO_DRIFT_2026-07-28.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/entry/RELAY_ENUM_DRIFT_2026-07-28.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/R1_RELAY_ENUM_AND_SECTION_CHECK_STATIC.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/R1_CLAUDE_REVIEW_2026-07-27.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/EXACT_STOCK_RUNTIME_BASELINE_2026-07-27.md"
harness_contract: "static entry-ack closure pinned; paired select-role-to-scene-ready scenario remains runtime-pending"
evidence_state: "runtime-observed for exact stock; candidate rebuilt and deployed, selected-role oracle pending"
runtime_constraint: "the previous VPS plain-path executable was not stock, so all candidate diagnosis must restart from a hash-pinned paired arm"
candidate_baseline: "canonical P3 1516b8b plus byte-identical 11-file B1--B4 overlay; native-x86 candidate and runtime acceptance remain separate"
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-28T09:55:00Z"
next_action: "Use the clean V6 stock/candidate Journey bundle to close the role-load parser checkpoint: materialize target/candidate V0..V6 item-block evidence into a machine-readable artifact and add a truthful Journey matcher; do not claim enter-world from the 7-rung public handoff."
---

# Feature Card: PLY-ENTER-WORLD — select role to scene-ready

## 1. Outcome

### Capability

After selecting an existing role, the client enters a scene with target-backed
ready confirmation and the bounded initial player/NPC/doodad synchronization.

### Observable result

- [ ] The selected role crosses the target-backed entry milestones and reaches
      the chosen scene-ready state.
- [ ] The selected initial synchronization set is received/handled in the
      target-backed order without an unexplained disconnect or generic UI error.

### Explicit non-goals

- Player movement, combat, NPC AI semantics, loot, broad reconnect, or all
  initial S2C packet-body parity.
- Role creation and unrelated Gateway/Paysys behavior unless target evidence
  proves an unavoidable dependency.

## 2. Context and dependencies

### Why this slice now

This is the first player-visible outcome. It joins the accepted foundations
but remains one card because selection, ready, and initial sync have one
observable transition and failure domain. Static research may proceed before
all foundations are accepted; implementation and acceptance may not.

### Prerequisite Feature Cards

| Feature ID | Required result | Status at start |
|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | accepted target-bounded boot contract | researching |
| `FND-CLIENT-GS-SECURE-TRANSPORT` | accepted first GS packet/stream contract | researching |
| `FND-RELAY-ROLE-ENVELOPE` | accepted active role envelope contract | researching |
| `FND-ROLE-LOAD-SPINE` | accepted selected role-load contract | researching |

### State-machine boundary

```text
existing role selected → role-entry/ready transitions → scene-ready → bounded initial object synchronization
```

### Consumers and downstream features

| Consumer / next feature | Relation | Impact if wrong |
|---|---|---|
| `PLY-MOVE-AND-POSITION-SYNC` | needs ready player/scene | movement evidence invalid |
| `NPC-SPAWN-AND-STANDARD-AI` | needs bounded scene/object state | NPC startup ambiguous |
| combat/interaction cards | require player in scene | no valid gameplay oracle |

## 3. Target contract (v2.5 authority)

### Target artifacts

| Artifact | SHA-256 | Authority for | Notes |
|---|---|---|---|
| `SO3GameServerD` | `47716c73…51124b94a` | target handlers/types/call closure | target DWARF |
| `SO3GameServer` | `3002bf4a…5c416f11` | paired runtime behavior | stripped stock |

### Target roots

| Domain | Exact identity | Required fact | Evidence ID / raw artifact |
|---|---|---|---|
| Function | `KPlayerServer::OnApplyCharacter @0x08061eb0` | client object-apply acknowledgement | entry dossier |
| Function | `KPlayerServer::OnClientConfirmReady @0x08079dfc`, `KPlayer::OnClientReady @0x0839f87e`, `OnSyncNew*Respond` | ready gate and initial-sync acknowledgements | entry dossier |
| Type / field | `C2S_APPLY_CHARACTER`, `C2S_SYNC_NEW_*_RESPOND`, target KPlayer sync fields | required wire offsets and acknowledgement state | entry dossier; target producer/state transition still open |
| Protocol | `c2s_client_confirm_ready=2`, `c2s_apply_character=183`, response routes `188..190` | direction/size/handler/fields | entry dossier; paired capture still required |
| Save/load | role-to-scene consumer boundary | required loaded state only | depends on role-load card |

### Target behaviour / invariants

1. Each named entry milestone must be observable in both stock and candidate;
   a client string alone is not an oracle.
2. The card may defer initial packets only when they are proven outside the
   selected scene-ready invariant and assigned to a later card.

## 4. Origin comparison and Graph dossier

### Required read-only dossiers

```text
compare-engine/tools/query/evidence_query.py function KPlayerServer::OnApplyCharacter
compare-engine/tools/query/evidence_query.py function KPlayerServer::OnClientConfirmReady
compare-engine/tools/query/evidence_query.py protocol c2s_client_confirm_ready
compare-engine/tools/query/graph_query.py callees KPlayer::OnClientReady
```

### Evidence classification

| Class | Claim / observation | Raw evidence location | Consumer / decision |
|---|---|---|---|
| Confirmed fact | target entry acknowledgement handler identities, direct callees, five wire type layouts, and source absence are recorded separately | entry dossier + raw target/source observations | static port closure |
| Inference | missing candidate acknowledgement routes plausibly explain post-selection entry failure | source absence plus target target-only routes | must be disproved by paired capture |
| Unresolved / conflict | route order, `m_bExtDataLoadFinish` producer, transition into `gsWaitForSyncClientData`, full initial sync set | target constructor/entry decompile + paired capture | blocks write lease and acceptance |

### Unknown policy

| Unknown / conflict | On required path? | Resolution evidence needed | Decision |
|---|---:|---|---|
| foundation contracts not accepted | yes | accepted prerequisite cards | block implementation/acceptance; static research continues |
| target ready/initial-sync packet contract | yes | decompile + paired capture | block |
| optional initial object packet | no if target path proves optional | target caller/consumer evidence | defer to named future card |

## 5. Boundary matrix

| Boundary | Required contract | Candidate implementation point | Verification |
|---|---|---|---|
| C++ logic | selection/ready/scene state machine | pending | target decompile + milestones |
| ABI/layout | selected player/scene payload fields | pending | DWARF |
| Protocol/transport | entry/ready/initial synchronization routes | pending | paired capture |
| Persistence | required role state consumer | inherited role-load contract | role-state observation |
| Lua | only if target closure invokes it | pending | actual call/return evidence |
| Table/config | selected map/scene inputs if reached | pending | target reader + startup evidence |

## 6. Candidate implementation scope

B1 has a narrow write lease only: account request/chunk/final routes, account
buffer lifecycle and target-backed account block parsing. B2–B4 remain separate
ordered overlays. Forbidden: bypassing a ready gate, injecting generic success
packets, accepting unknown packet bodies, or folding movement/combat/AI work
into entry.

The current isolated `w1-entry-overlay` materializes the historical 11-file
B1--B4 patch as one provenance-preserving working copy. This does **not** merge
their acceptance: each B1–B4 contract, reviewer verdict, build, and runtime
measurement remains independently open.

## 7. Acceptance checklist

- [ ] All four prerequisite cards are accepted.
- [x] Target entry/ready/initial-sync acknowledgement closure and evidence package are recorded; target state producer/order remains open.
- [ ] Scope explicitly classifies every observed initial packet as required or deferred.
- [x] Cross-model review completed and returned `REQUEST_CHANGES`; see
  `evidence/B1_B4_CLAUDE_REVIEW_2026-07-26.md`.
- [x] B1 isolated candidate builds native x86; it is not an active/deployed
      candidate and does not satisfy integrated provenance.
- [ ] Paired existing-role scenario reaches the named scene-ready and bounded sync milestones.
- [ ] Rollback and no-regression evidence are retained.

## 8. Must-stay-unchanged invariants

| Invariant | Baseline evidence | Candidate result | Verdict |
|---|---|---|---|
| Foundation card contracts remain satisfied. | their acceptance bundles | pending | pending |
| Stock executable, role seed, and unrelated scene paths remain preserved. | capture/data IDs pending | pending | pending |

## 9. Deferrals, risks and handoff

Movement, broad object-stream parity, combat, and reconnect hardening are
explicitly deferred to their roadmap cards. Handoff includes source/build
provenance, target evidence, paired runtime bundle metadata, reviewer verdict,
rollback artifact, and the next feature frontier.
