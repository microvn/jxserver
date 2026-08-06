---
feature_id: "FND-RELAY-ROLE-ENVELOPE"
title: "GameServer to Center role-section envelope"
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "researching"
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
  - "SO3GameCenterD sha256:655b28126b972d1b3e1b3c02b827356c4d2ad6cf421b258152b130ec3c214d79"
evidence_package: "linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/relay/RELAY_ROLE_ENVELOPE_DOSSIER.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/relay/CONFIRM_PLAYER_LOGIN_REQUEST_DRIFT_2026-07-28.md; linux-build/docs/waves/wave-2/CONSUMER_INVENTORY.md; linux-build/docs/waves/wave-3/STATE.md"
harness_contract: "pending paired Relay packet/role-section observation"
evidence_state: "static-confirmed — target confirm envelope is 10 bytes with dwPlayerID/dwIP; candidate sends 6 bytes; Center permit response remains unproven"
runtime_constraint: "A paired GS↔Center capture after the target-sized confirm overlay is required; current candidate stops after handshake/ping."
candidate_baseline: "git 4e60f1697bf71f2dd46ff390f24f3a6a357e300f; manifest required before edit"
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-28T00:00:00Z"
next_action: "Identify the candidate numeric client-IP owner, then prepare a two-file target-sized confirm envelope overlay for opposite-model review; do not batch role-section changes."
---

# Feature Card: FND-RELAY-ROLE-ENVELOPE — GS/Center role envelope

## 1. Outcome

### Capability

GameServer and Center preserve the required role-section and Relay envelope
contracts on the active entry path.

### Observable result

- [ ] The selected role-envelope path reaches its target-backed completion
      milestone without silently discarding a required section/sentinel.
- [ ] Unknown role sections are preserved or explicitly target-proven
      irrelevant on this path; they are never parsed by guesswork.

### Explicit non-goals

- Full Center/Gateway/social protocol coverage.
- Save/relogin semantic parity beyond the envelope needed by this Wave.

## 2. Context and dependencies

### Why this slice now

Entry consumes role data through Relay. A role list or TCP connection does not
prove that the envelope's sections, order, sizes, and completion state are
correct. This is a shared spine, deliberately separate from a player class.

### Prerequisite Feature Cards

| Feature ID | Required result | Status at start |
|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | GS starts its Relay client with named configuration/providers | researching |

### State-machine boundary

```text
Center role section emitted → Relay envelope received → section consumers complete → entry may request role load
```

### Consumers and downstream features

| Consumer / next feature | Relation | Impact if wrong |
|---|---|---|
| `FND-ROLE-LOAD-SPINE` | consumes base/state sections | shifted role state or refusal to load |
| `PLY-ENTER-WORLD` | consumes completion gate | entry denial or generic client failure |
| `PLY-SAVE-LOGOUT-RELOGIN` | later reuses envelope | persistence corruption risk |

## 3. Target contract (v2.5 authority)

### Target artifacts

| Artifact | SHA-256 | Authority for | Notes |
|---|---|---|---|
| `SO3GameServerD` | `47716c73…51124b94a` | role types/layout and named Relay consumers | target DWARF |
| `SO3GameCenterD` | `655b2812…3c214d79` | peer-side role-login producer and wire type | target DWARF |

### Target roots

| Domain | Exact identity | Required fact | Evidence ID / raw artifact |
|---|---|---|---|
| Function | `KGameServer::DoPlayerLoginRequest@0x08084436`, `KGameServer::OnPlayerLoginRespond@0x0807ea58` | Center peer producer/response consumer | Relay dossier |
| Type / field | role sections and flags | offset/size/type/order | pending DWARF evidence |
| Protocol | `R2S_PLAYER_LOGIN_REQUEST` size `0x12e`, `BaseInfo@0x82`; response size `0x26` | typed payload boundary; wire framing remains open | Relay dossier + paired capture pending |
| Save/load | envelope-to-role boundary | opaque/preserved blocks and versioning | pending consumer evidence |

### Target behaviour / invariants

1. A completion sentinel is a state-machine fact, not a generic no-op route.
2. Payload size, field layout, and section order each require target evidence.

## 4. Origin comparison and Graph dossier

### Required read-only dossiers

```text
compare-engine/tools/query/evidence_query.py function KRelayClient::OnPlayerLoginRequest
After the handler closure identifies the actual envelope type and route, record
the exact `evidence_query.py type` and `evidence_query.py protocol` commands.
```

### Evidence classification

| Class | Claim / observation | Raw evidence location | Consumer / decision |
|---|---|---|---|
| Confirmed fact | GameServer target artifact identity is pinned. | artifact hash | scope anchor |
| Inference | historical routing fixes may identify a missing sentinel. | historical state only | must be re-proven |
| Unresolved / conflict | exact peer artifact, route IDs, section layout/order, completion consumer | pending dossier/capture | blocks implementation |

### Unknown policy

| Unknown / conflict | On required path? | Resolution evidence needed | Decision |
|---|---:|---|---|
| role-section offset/order | yes | DWARF + target consumer | block |
| Relay route/size/handler | yes | decompile + paired capture | block |

## 5. Boundary matrix

| Boundary | Required contract | Candidate implementation point | Verification |
|---|---|---|---|
| C++ logic | handler/state completion order | pending | target decompile |
| ABI/layout | role types/sections | pending | DWARF |
| Protocol/transport | Relay direction/size/route | pending | capture + target route |
| Persistence | opaque section preservation | pending | round-trip/DB observation |
| Build/DSO | Relay provider inputs if used | pending | build manifest |

Lua and table/config are `N/A` unless a target consumer proves them on path.

## 6. Candidate implementation scope

No write lease exists. Forbidden until contract-ready: changing a role struct
from source order alone, accepting unknown sections permissively, mapping a
route to a no-op, or changing Relay packet size without target evidence.

## 7. Acceptance checklist

- [ ] Target GS and peer artifacts/route evidence are pinned.
- [ ] Required role section offsets/order and completion consumers are proven.
- [ ] Paired stock/candidate envelope observation records packet/state results.
- [ ] Cross-model review, baseline, native build, and active binary provenance pass.
- [ ] Required opaque/unknown section policy is verified, not assumed.
- [ ] Rollback is retained.

## 8. Must-stay-unchanged invariants

| Invariant | Baseline evidence | Candidate result | Verdict |
|---|---|---|---|
| Unowned Relay routes and role sections remain unchanged. | pending route inventory | pending | pending |

## 9. Deferrals, risks and handoff

Cross-GS/social and full persistence semantics are deferred. Handoff records
both binary origins separately, layout and capture evidence, provenance,
review, rollback, and the exact next consumer to research.
