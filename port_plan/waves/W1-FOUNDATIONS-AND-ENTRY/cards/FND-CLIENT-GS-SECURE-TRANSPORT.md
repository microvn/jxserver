---
feature_id: "FND-CLIENT-GS-SECURE-TRANSPORT"
title: "Stock-compatible client to GameServer secure transport"
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "reviewing"
priority: "P0"
kind: "foundation"
owner: "orchestrator"
producer_model: "codex"
reviewer_model: "claude"
planned_producer_model: "codex"
planned_reviewer_model: "claude"
graph_db_sha256: "25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba"
target_artifacts:
  - "SO3GameServerD sha256:47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a"
  - "SO3GameServer sha256:3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11"
evidence_package: "linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/CLAUDE_KSG_CODEC_REVIEW_2026-07-27.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/transport/KPLAYER_CHECKPACKAGE_DRIFT_2026-07-28.md; linux-build/port_plan/waves/W1-FOUNDATIONS-AND-ENTRY/evidence/relay/CONFIRM_PLAYER_LOGIN_REQUEST_DRIFT_2026-07-28.md"
harness_contract: "retain paired raw pcap and replay harness; compare receive frame boundary, helper selection, call count, and decoded route"
evidence_state: "runtime-confirmed handshake/ping validator pass; client stops before proto 3, with target/candidate relay-confirm envelope drift now pinned"
runtime_constraint: "Relay 5003 permit response must be captured after the 10-byte envelope overlay; current candidate has not proven OnConfirmPlayerLoginRespond."
candidate_baseline: "git 4e60f1697bf71f2dd46ff390f24f3a6a357e300f; manifest required before edit"
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-28T00:00:00Z"
next_action: "Open the relay-confirm envelope card: identify candidate numeric client-IP ownership and review a target-sized 10-byte confirm request before rebuilding."
---

# Feature Card: FND-CLIENT-GS-SECURE-TRANSPORT — client/GS transport

## 1. Outcome

### Capability

The client and GameServer complete the target-compatible stream/security setup
and the candidate decodes the first client packet on the active entry path.

### Observable result

- [ ] The selected first client-to-GS packet has the target-backed decoded
      identity, length/framing, and receiving route.
- [ ] The candidate does not close or misroute the stream due to a guessed
      pass-through socket/crypto implementation.

### Explicit non-goals

- Full protocol coverage or all gameplay packet body parity.
- Client binary patching or Gateway authentication behavior.

## 2. Context and dependencies

### Why this slice now

Claude disproved the proposed KSG word-order mismatch: target and candidate
codec semantics are identical. The next bounded investigation is receive
framing and decode-helper selection, with raw artifacts retained for review.

### Prerequisite Feature Cards

| Feature ID | Required result | Status at start |
|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | target-bounded GS can boot with selected providers/config | researching |

### State-machine boundary

```text
client connects to GS → target stream/security setup → first framed packet → named GS receiving route
```

### Consumers and downstream features

| Consumer / next feature | Relation | Impact if wrong |
|---|---|---|
| `PLY-ENTER-WORLD` | sends ready/entry traffic over this boundary | generic disconnect/UI error masks root cause |
| every later client feature | reuses transport framing/security | packet-level evidence is invalid |

## 3. Target contract (v2.5 authority)

### Target artifacts

| Artifact | SHA-256 | Authority for | Notes |
|---|---|---|---|
| `SO3GameServerD` | `47716c73…51124b94a` | stream/provider symbols and receiving handlers | target DWARF |
| `SO3GameServer` | `3002bf4a…5c416f11` | paired target runtime arm | stripped stock |

### Target roots

| Domain | Exact identity | Required fact | Evidence ID / raw artifact |
|---|---|---|---|
| Function | `KG_AsyncSocketStream::Init`, `Recv`, `KSG_DecodeEncode` | setup, state placement, receive framing/helper selection | `CLAUDE_KSG_CODEC_REVIEW_2026-07-27.md` |
| Protocol | first client packet | direction, wire framing, decoded ID, handler | same capture; post-repair route still required |
| Transport / DSO | `libcommon.a(KG_Socket.o)` | security provider and table-code owner | same dossier |

### Target behaviour / invariants

1. Packet bytes, framing, decode, and dispatch are distinct claims and must
   not be collapsed into one symbolic protocol name.
2. A successful TCP connection is insufficient; the named first decoded route
   must be observed in both arms.

## 4. Origin comparison and Graph dossier

### Required read-only dossiers

```text
Pending target-root selection: record the exact `evidence_query.py protocol`,
`evidence_query.py function`, and `graph_query.py callers` commands only after
the stock capture identifies the first decoded route/provider/receiver.
```

### Evidence classification

| Class | Claim / observation | Raw evidence location | Consumer / decision |
|---|---|---|---|
| Confirmed fact | target executable identities are pinned | artifact hashes | scope anchor |
| Inference | historical pass-through socket is suspect | historical runtime note | guide for falsification only |
| Unresolved / conflict | provider ABI, framing, decode and route | pending target dossier/capture | blocks implementation |

### Unknown policy

| Unknown / conflict | On required path? | Resolution evidence needed | Decision |
|---|---:|---|---|
| target security provider and ABI | yes | imports/DSO/decompile + build evidence | block |
| target first-packet decode/route | yes | paired capture + target handler evidence | block |

## 5. Boundary matrix

| Boundary | Required contract | Candidate implementation point | Verification |
|---|---|---|---|
| C++ logic | decode/dispatch sequence | pending | target decompile |
| ABI/layout | provider/vtable/object boundary if present | pending | DWARF/import inspection |
| Protocol/transport | framing, security, ID, first handler | pending | paired capture |
| Build/DSO | real provider and link order | pending | build manifest |

Lua, table/config, and persistence are `N/A` unless the evidence closure proves
they affect this first-packet boundary.

## 6. Candidate implementation scope

No source or DSO write lease exists. In particular, do not restore a
pass-through `kg_socket` path, hard-code a decoded ID, or change packet ranges
until the target provider and route are proven.

## 7. Acceptance checklist

- [x] Target provider and codec parity were independently audited; decoded route and receive framing are still pending.
- [ ] Stock-vs-candidate paired capture has identical preconditions and normalization.
- [x] Claude cross-model review rejected the unsafe codec patch and recorded codec parity.
- [ ] Baseline/build/active-process provenance passes.
- [ ] First target-backed decoded route reaches the candidate handler.
- [ ] Rollback of candidate provider/binary is recorded.

## 8. Must-stay-unchanged invariants

| Invariant | Baseline evidence | Candidate result | Verdict |
|---|---|---|---|
| Stock connection behavior and target binaries remain untouched. | target hashes | pending | pending |

## 9. Deferrals, risks and handoff

All later packet-body semantics, Gateway auth, and broad reconnect hardening
remain outside this card. Handoff must retain raw captures (private location),
their hashes/metadata, target decompile evidence, build input/provider identity,
review verdict, and next action.
