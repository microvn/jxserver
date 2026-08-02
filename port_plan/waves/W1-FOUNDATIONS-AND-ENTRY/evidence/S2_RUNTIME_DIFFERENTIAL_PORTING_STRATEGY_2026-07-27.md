# S2 — Runtime-differential porting strategy

Status: research point; no workflow or source change is authorized by this note.

## Thesis

The current plan is too waterfall-shaped: it attempts to complete broad static
closures before using the live journey to say which closure matters.  For a
binary-backed C++ port, runtime should select the next port boundary; target
evidence should make that boundary safe to patch; acceptance remains strict.

## Three layers

### 1. Reusable lab, built once

Maintain one hash-pinned stock/candidate harness: same topology, config/data/
schema/role seed, active-process hash, rollback, and fixed capture points.
For entry the checkpoints are, for example:

```text
connect -> security key -> first decoded GS packet -> OnPlayerLoginRequest
-> LoadBaseInfo -> GetScene -> role sections -> ready -> gsPlaying
```

This is a shared operational asset, not a new dossier per card.

### 2. One observed fault slice per iteration

```text
paired stock/candidate probe
  -> last shared + first divergent checkpoint
  -> target evidence for that boundary only
  -> smallest vertical source slice
  -> opposite-model review
  -> provenance build + identical paired probe
```

Every iteration must produce exactly one of: a later shared checkpoint, a
falsified hypothesis, or a named unavailable-oracle blocker.  If it produces
none, stop rather than expanding documentation.

### 3. Acceptance after diagnosis

Diagnosis permission is not acceptance permission.  A reversible paired probe
may run before all foundation cards are accepted.  A source patch still needs
target evidence, isolated ownership, review, build provenance, rollback, and
the paired oracle required by its card.

## Operating rules

- Do not patch account-data, ready ACK, initial sync, and transport together.
  The first divergent checkpoint owns the next patch.
- Do not require a foundation to be accepted before using the integrated
  journey to observe it.  Keep prerequisite acceptance for feature claims.
- An evidence capsule links raw capture/query/decompile outputs and lists only
  the semantic decisions in the current diff.  It does not repeat full card,
  Wave, or Atlas narratives.
- Scale agents only after a checkpoint selects disjoint work: one producer/
  runtime driver; one opposite-family reviewer; at most one analyst for the
  current target closure.
- Promote documentation only at real gates: patch, review, build, runtime,
  blocker, or handoff.  Ordinary queries remain raw artifacts plus a compact
  evidence row.

## W1 immediate interpretation

Run a clean paired selected-role probe and retain its raw boundary evidence.
Then choose one branch only:

- if candidate first diverges at `GetScene`, investigate the topology/data
  owner and `LoadBaseInfo -> GetScene` contract;
- if it diverges earlier, investigate only the first packet receive/framing/
  dispatch closure;
- do not reopen downstream account/ready/initial-sync work until the earlier
  checkpoint is shared.
