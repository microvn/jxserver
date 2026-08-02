---
wave_id: "W1-FOUNDATIONS-AND-ENTRY"
status: "active"
roadmap_band: "T1"
baseline: "4e60f1697bf71f2dd46ff390f24f3a6a357e300f; implementation baseline must be re-pinned per card"
integrated_scenario: "Boot the rebuilt GameServer, establish the stock-compatible client/GS and Center/GS contracts, select one existing role, and reach scene-ready with the bounded initial synchronization."
created_utc: "2026-07-24T16:06:10Z"
updated_utc: "2026-07-28T00:00:00Z"
next_action: "Open the relay-confirm envelope card: identify candidate numeric client-IP ownership and review a target-sized 10-byte request before rebuilding."
---

# Wave 1 — Foundations and Entry

## Purpose and status

This is the first **planned** Wave in the feature-led plan. It is not a
replacement for the historical `wave-1b`, `wave-2`, or `wave-3` records, and
it makes no claim that their output is accepted. Those records are evidence
leads which each card must revalidate against current target artifacts.

The Wave groups the minimum contracts needed for one visible journey. It does
not split the journey by source file or by every intermediate packet. The
actual source edits, build, reviewer verdict, and runtime captures belong in
the owning Feature Card.

## Entry contract

- The five cards below retain independent target evidence and an explicit
  unknown policy.
- No producer may edit the canonical source tree from this Wave manifest.
- A card may enter `implementing` only after its own `contract-ready` gate,
  baseline manifest, write lease, and opposite-model reviewer assignment.
- The role used for the final journey must be a reproducible existing-role
  seed. Creating a role is outside this Wave unless target evidence proves it
  is an unavoidable prerequisite.

## Required Feature Cards

| ID | Capability | Card status | Why required for this scenario |
|---|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | Target-bounded GameServer initialization | reviewing — exact stock boot and enter-world arm proven; candidate provenance/review open | Supplies data, scripts, DSO, locale, map, and configuration prerequisites. |
| `FND-CLIENT-GS-SECURE-TRANSPORT` | Stock-compatible client/GS security and first-packet decoding | reviewing — handshake/ping pass; relay-confirm 6-vs-10-byte drift is target-pinned and needs its own bounded overlay | A role cannot enter a scene if the stream/framing boundary changes its first GS packet. |
| `FND-RELAY-ROLE-ENVELOPE` | GS/Center role-section envelope on the active path | researching — target Center type/route pinned; selected-role capture deferred by environment | Supplies the role data and completion signals consumed by entry. |
| `FND-ROLE-LOAD-SPINE` | Versioned role base/state loading without downstream shift | researching — target parser/layout dossier saved; block-version delivery unproven | Ensures the selected role reaches entry with a target-compatible state layout. |
| `PLY-ENTER-WORLD` | Select role, scene-ready, initial bounded synchronization | researching — exact stock passes the visible journey; previous VPS "stock" failures are invalid baseline evidence | This is the observable Wave outcome. |

## Current operating constraint

A runnable client is available.  The exact-stock arm now proves the visible
journey, but no candidate may inherit that result.  The next client session
must run one hash-pinned candidate arm under the same topology and record the
first divergent milestone.

## Integrated scenario and exit contract

```text
Preconditions:
  - a target-compatible configuration/data set and a reproducible existing-role seed;
  - stock and candidate executions record their binary/config/data/schema identities;
  - all five cards are accepted individually.

Input:
  1. Start the selected GS/Center/Gateway topology.
  2. Authenticate through the normal client path.
  3. Select the seeded role and accept the transfer/ready sequence.

Expected result:
  The candidate reaches the named target-backed milestones for boot, first GS
  packet, Relay role completion, role load, scene-ready, and the selected
  initial player/NPC/doodad synchronization. The card dossiers define the
  exact packet/state/log/DB observations; this manifest does not invent them.

Must stay unchanged:
  Stock binaries remain untouched; unrelated maps, role blocks, protocol
  routes, and current accepted source closure remain unchanged unless an
  owning card explicitly proves and leases them.
```

## Integration order

1. `FND-RUNTIME-INITIALIZATION` establishes an honest cold-boot baseline.
2. `FND-CLIENT-GS-SECURE-TRANSPORT` and `FND-RELAY-ROLE-ENVELOPE` may research
   in parallel after the initialization inputs are identified; implementation
   remains serial where they share a file or binary boundary.
3. `FND-ROLE-LOAD-SPINE` consumes the accepted Relay envelope.
4. `PLY-ENTER-WORLD` consumes the accepted foundations and proves the visible
   scenario. It does not absorb movement, combat, broad reconnect, or full
   S2C-body parity.

## Wave acceptance

This Wave may become `accepted` only when every required card is `accepted`,
the integrated scenario above passes on recorded stock and candidate arms, and
the active candidate executable hash is recorded. A clean process boot, role
list, or generic client UI message is not Wave acceptance.

## Blockers and next frontier

| Blocker | Owning card | Resolution evidence | Effect on roadmap |
|---|---|---|---|
| Source/build provenance and opposite-model review remain open for the observed loader/DSO fingerprint | `FND-RUNTIME-INITIALIZATION` | card-scoped build-input manifest and reviewer verdict | no initialization completion claim |
| First client packet framing/security contract not yet pinned | `FND-CLIENT-GS-SECURE-TRANSPORT` | target socket/decompile plus paired capture | blocks role-entry implementation |
| Role envelope / role-section layout is unresolved | `FND-RELAY-ROLE-ENVELOPE` | DWARF, target route, and round-trip evidence | blocks role-load claim |
| Versioned role-load closure not yet pinned | `FND-ROLE-LOAD-SPINE` | DWARF plus target load consumer evidence | blocks entry implementation |
| Initial sync boundaries not yet target-backed | `PLY-ENTER-WORLD` | target handler/type/packet evidence and paired scenario | blocks Wave acceptance |
