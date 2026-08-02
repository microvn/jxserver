# Agent Team Workflow for JX3 Porting

This document defines how Claude Code and Codex collaborate on a bounded JX3
port feature. It is a coordination and quality-gate guide; it does not define
the content or order of Waves. See
[`../../port_plan/WAVES.md`](../../port_plan/WAVES.md) for Wave scope and
acceptance milestones.

## Objective

Use parallel agents to shorten reverse engineering and implementation time
without weakening correctness at ABI, protocol, Lua, DSO, persistence, or
runtime boundaries.

The desired flow is:

```text
evidence collection
  -> bounded feature specification
  -> isolated implementation
  -> independent cross-model review
  -> fresh build and artifact proof
  -> bounded stock-vs-candidate oracle
  -> accepted integration change
```

No model is allowed to turn a missing target fact into an implementation guess.

## Roles

| Role | Responsibility | May change production source? |
|---|---|---:|
| Coordinator | Choose the Feature Slice, issue task packets, manage leases, merge queue, and acceptance status. | Normally no |
| Evidence analyst | Obtain raw target/source evidence from DWARF, decompile, binary, GraphEngine, and data; separate facts, inferences, and unresolved questions. | No |
| Producer | Implement only the accepted feature scope in an isolated worktree. | Yes |
| Verifier | Independently reproduce critical evidence, review diff, run gates, and classify mismatches. | No patch changes |
| Runtime verifier | Own the declared harness scenario and its stock/candidate measurements when runtime evidence is required. | No production patch changes |
| Integrator | Applies accepted commits to the integration queue and runs post-merge gates. | Integration only |

These are capabilities, not permanent model personas: Claude or Codex may take
any role. The Coordinator may also be Integrator, and the Verifier may also run
a small runtime harness. The Producer must not be the sole evidence authority
and accepting verifier for its own patch.

## Cross-model rule

Use flexible model assignment, but require model separation at review:

```text
Claude oracle -> Codex producer -> Claude verifier
Codex oracle  -> Claude producer -> Codex verifier
```

For high-risk contracts, the verifier must independently inspect a target sample
(for example, a DWARF layout or target dispatch callsite) rather than only
accepting the oracle report.

## Concurrency and isolation

Normal operating capacity is one Coordinator, one or two writers, two or three
read-only analysts, one verifier, and one build/runtime runner. More writers do
not increase throughput when they contend on Player, Relay, protocol headers, or
persistence.

Each writer receives an isolated Git worktree and one ticket. Do not share:

- object directories or output binaries;
- mutable deploy roots, logs, or database instances;
- generated shims or temporary protocol captures; or
- ownership of a hotspot file.

Typical commands are:

```bash
# Claude Code writer
claude --worktree <ticket> --bg --name <ticket> "<task-packet>"

# Codex writer
git worktree add <worktree-path> -b task/<ticket> <base-sha>
codex exec -C <worktree-path> -s workspace-write "<task-packet>"
```

Use agent teams for independent research/review only when they have no shared
write ownership. Worktree sessions remain the default for production edits.

## Baseline and leases

Every task packet declares an exact committed `BASE_SHA`. A worktree does not
contain uncommitted changes from another worktree; never silently rely on a dirty
main worktree as a baseline.

One writer at a time owns a hotspot. Typical hotspots are player lifecycle,
Relay/login, world initialization, Lua registration, protocol headers, role-data
definitions, and compatibility shims. Feature work should split into:

1. a module commit in subsystem-owned files; then
2. a small glue commit through the hotspot owner.

The Coordinator returns a patch to the Producer when it changes a leased path
without an explicit handoff.

## Required task packet

Every producer receives the following minimum contract:

```text
ROLE / TICKET / OBJECTIVE
BASE_SHA / WORKTREE / TIMEBOX
FEATURE_CARD / DEPENDENCIES
OWNED_PATHS / LEASED_HOTSPOTS / FORBIDDEN_PATHS

TARGET_ARTIFACTS / RAW_ORACLE
GRAPH_DB / GRAPH_SNAPSHOT_SHA256
GRAPH_QUERIES / GRAPH_RESULT_FILES
REQUIRED_EVIDENCE_STATUS / ACCEPTED_UNKNOWNS

EXPECTED_DELIVERABLES
BUILD_COMMANDS / RUNTIME_SCENARIO
```

The task packet must also state: do not use old plan/chat conclusions as target
evidence; do not reformat/re-encode unrelated files; do not consume artifacts
from another worktree; and stop when a new contract/hotspot lease is needed.

### Evidence package

The Coordinator attaches a compact, reopenable evidence package to the task
packet. It is the worker's starting context, not a summary that replaces raw
artifacts:

```text
TARGET ARTIFACT SHA / evidence IDs / exact raw locations
SAVED GraphEngine queries and result files
CONFIRMED FACTS / INFERENCES / UNRESOLVED OR CONFLICTING CLAIMS
SOURCE-2010 and candidate observations, kept distinct from target facts
BOUNDARIES IN SCOPE: ABI, protocol, Lua, table, persistence, DSO, runtime
```

Give a worker only the context needed for its owned decision, but never hide
the target evidence, baseline, raw-diff command, or acceptance contract needed
to verify that decision. Scoped context prevents chat-history bloat; it is not
permission to make the worker blind to evidence.

## GraphEngine gate

Before changing a feature that has reverse-engineering or contract risk, the
Producer records the relevant read-only queries from
[`../tools/query/`](../tools/query/):

- function dossier and target caller/callee closure;
- type dossier for layout/role-block boundaries;
- protocol dossier for routes and payload evidence;
- Lua or table dossier when that boundary is used; and
- coverage/snapshot health when provenance matters.

GraphEngine results are scoped evidence, not a parity oracle:

- `TARGET_STATIC_CONFIRMED` may support a static claim when origin, artifact hash,
  address/file location, and evidence ID are present.
- `PROVISIONAL`, `partial`, and `UNRESOLVED` expand research scope; they do not
  authorize a correctness claim.
- Runtime behavior, byte round-trip, and user-visible acceptance still require
  the appropriate build/runtime oracle.

Only the GraphEngine refresh owner runs extraction, lanes, ingestion, or SQLite
mutation. Producers and verifiers use the read-only query tools.

## Producer and verifier loop

1. Evidence analyst provides facts, alternatives, and unknowns in the evidence
   package.
2. Coordinator writes a Feature Card and task packet.
3. Producer implements only the stated closure and self-reviews the diff.
4. Opposite-model verifier replays critical target evidence, reviews scope and
   invariants, and runs the declared gates.
5. A mismatch returns to the correct owner; it is never silently repaired by
   the reviewer.
6. Integrator accepts only a passing, provenance-complete handoff.

The verifier reports `pass/fail`, evidence checked, mismatch severity, repro
steps, suspected boundary, and a recommendation. It does not silently repair a
producer patch while reviewing it.

### Mismatch routing

| Finding | Owner for the next action |
|---|---|
| Missing, contradictory, or insufficient target evidence | Evidence analyst |
| Candidate diff violates the accepted contract | Producer |
| Scenario, capture, or normalization is invalid | Runtime verifier |
| Scope, dependency, lease, or acceptance condition is ambiguous | Coordinator |
| Conflict in a packed layout, protocol/transport, Relay, or Player serialization spine | Hotspot owner, with fresh target evidence |

## Static and runtime gates

Static acceptance proves the named target contract: artifact provenance,
layout/enum facts, call or reader closure, binding shape, and candidate scope.
It is sufficient only for a card whose declared outcome is static.

Runtime acceptance proves behavior. The card declares a small harness contract:
scenario input, stock and candidate arms, capture points, permitted
normalization, expected state/packet/log/DB delta, and rollback condition. A
golden snapshot is optional: use it only when the stock scenario is stable and
the capture can be reproduced. Do not manufacture a snapshot merely to satisfy
process.

Fresh builds must use a new object/output directory, fail on compile or link
error, publish only after a successful link, and record source SHA, compiler
environment, DSO hashes, and binary hash.

For behavioral acceptance, compare stock and rebuilt arms with identical
data/config/schema inputs where possible. A bounded scenario captures the
relevant packet boundary, logs, process status, DB/role-state change, and active
artifact identity. Normalize only proven nondeterminism.

Do not interpret a successful process start, a TCP accept, a clean-looking log,
or a matching symbol as runtime acceptance.

## Handoff and merge queue

The handoff includes:

```text
ticket, base/head SHA, commits, changed paths, owned/leased paths,
target evidence IDs, GraphEngine snapshot/query result files,
commands and exit codes, build metadata, artifact hash,
test/runtime results, accepted unknowns, and blockers.
```

Every role also returns this compact report, so that a new Claude/Codex session
can resume without reconstructing intent from chat:

```text
UNDERSTANDING: confirmed facts, inferences, unresolved items
PLAN: completed/next action and why
ARTIFACTS CHANGED: files, commits/diff, evidence or capture files
RISKS / UNKNOWNS: remaining claim, impact, owner, and blocking status
```

Integration order is:

```text
handoff -> scope/lease check -> evidence check -> cross-model review
-> rebase on queue head -> build/artifact gate -> runtime gate -> merge
```

If a conflict reaches a packed struct, protocol boundary, Relay, or Player
serialization spine, return it to the owner. The Integrator does not choose an
unverified side of a semantic conflict.

## Operational principles learned from early Waves

1. Prove ABI/layout before changing logic.
2. Treat binary provenance as an acceptance gate.
3. Debug the first divergent state transition and packet boundary, not generic
   client wording.
4. Start every task from a canonical accepted baseline plus a small overlay.
5. Run stock and rebuilt oracles in parallel when runtime acceptance is claimed.
6. Close a Wave only at its furthest proven milestone; do not mask an earlier
   contract failure with permissive logic or log suppression.
