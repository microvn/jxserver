# S4 — Runtime-first boundary porting experiment

**Status:** proposal for experiment; not yet an accepted workflow change.  
**Scope:** W1 (`boot -> login -> enter-world`).  
**Do not:** treat this note as authorization to change server source, deploy, or replace the current Wave skill.

## Problem observed

W1 accumulated substantial dossier, Atlas, card, and state work while producing
too few small, attributable source changes.  The current acceptance graph can
also delay the only useful diagnostic: a paired runtime journey that identifies
where stock and candidate first diverge.

The issue is not DWARF, decompile, or opposite-model review.  Those remain the
authority and safety mechanisms.  The issue is using manually maintained
planning artifacts as prerequisites for runtime diagnosis.

## Decision to test

Use the existing port map as a navigation map, not as an acceptance gate.

```
paired stock/candidate probe
  -> first divergent rung
  -> DWARF + decompile only around that contract boundary
  -> one atomic micro-patch
  -> executable contract oracle
  -> risk-tiered cross-model review
  -> rebuild and rerun the exact paired probe
```

### Definitions

- **Wave:** retained as the user-visible/release objective, e.g. W1 reaches
  `enter-world` under a defined oracle.
- **Feature roadmap:** retained as backlog and priority only.
- **Port map:** retained as a thin technical map that locates the likely owner,
  caller, and next runtime rung.  It must not require all mapped nodes to be
  researched or accepted before a probe runs.
- **Unit of work:** one *contract boundary*: one ABI/layout, enum/protocol ID,
  serialization, initialization, or state-transition decision plus its direct
  consumer/registration.
- **Boundary capsule:** a concise, append-only record of a real run and its
  resulting decision.  It replaces duplicated manual progress propagation.

## Required runtime assets

`jx3probe` is the proposed minimal checked-in harness.  For the same fixture,
it runs stock and candidate arms and records a machine-readable ledger:

```
rung | sequence | opcode | length | payload_sha256 | key/state_fingerprint
```

Initial rung ladder:

```
accept -> security key -> first decoded C2S -> OnPlayerLoginRequest
-> LoadBaseInfo -> GetScene -> role sections -> ready -> gsPlaying
```

Every arm must record provenance before comparison: executable hash, DWARF
hash, relevant DSO/config/data hashes, fixture/role seed, and client identity.
Raw captures and ledgers are retained.  A raw PCAP alone is not a replay oracle
when key exchange or codec state makes replay invalid.

## Contract oracle selection

| Contract kind | Required evidence/oracle |
| --- | --- |
| ABI/layout | DWARF-backed `sizeof`/`offsetof`/enum assertion |
| Codec or isolated logic | oracle-mirror or binary/candidate differential cases |
| Packet or DB blob | byte-level comparison, including consecutive frames where stateful |
| State transition | paired milestone trace plus state fingerprint |
| Init/DSO/runtime | provenance and process/DSO hash check |

Runtime progress chooses the scope; it does not by itself prove packet semantics.
DWARF and the appropriate contract oracle remain mandatory for that proof.

## Review policy

Opposite-model review is mandatory for serialized packets/blobs, ABI/layout,
enum or protocol IDs, initialization/state gates, and any patch not covered by
an executable oracle.  Local non-serialized changes with a strong paired oracle
may be reviewed asynchronously.

The reviewer must emit a named verdict artifact as its final action.  A missing
or incomplete transcript is not a verdict.

## Boundary capsule format

Each actual experiment adds one small block (or equivalent generated JSON) to
the boundary index:

```
boundary: transport.encode_decode_mode
run: <timestamp/id>
provenance: stock=<sha> candidate=<commit,binary sha> fixture=<id>
first_divergence: <rung/sequence/opcode>
target_contract: <DIE/address/value>
hypothesis: <falsifiable claim>
change: <commit or none>
oracle: <assertions/diff artifacts>
result: <advanced rung | rejected | blocked>
review: <verdict artifact or n/a>
next: <one next boundary>
```

Generated artifacts should include the raw per-arm ledger, comparison result,
provenance, build result, review verdict, and retained capture.  The index must
link to them instead of copying their contents into State/Card/Atlas documents.

## Pilot: W1 transport mode-4

**Candidate boundary:** `transport.encode_decode_mode`.

Existing evidence suggests the target `KPlayerServer::Init` selects the fast
encode/decode mode (value 4), while a candidate path may select mode 0.  This is
a *pilot hypothesis*, not an approved patch.

Preflight must establish all of the following before any source change:

1. The currently runnable candidate actually uses mode 0 on the compared path.
2. Target mode 4 belongs to the same live client/server transport state.
3. Stock and candidate runtime provenance hashes are valid and pinned.
4. A real client can perform the same fixture twice for paired capture.

Pilot success criteria:

1. The paired ledger runs in at most 15 minutes per arm pair.
2. Stock versus pre-patch candidate identifies the first divergent rung without
   manual log reading.
3. A boundary patch advances the candidate by at least one rung on the same
   probe, with artifacts retained.
4. The pilot completes within 6 hours, produces one small attributable commit,
   and adds no more than one page of human-written documentation.

If the first divergence cannot be measured within the 6-hour limit, stop
feature-port work.  The output becomes a bounded observability/harness blocker,
not another broad dossier.

## Migration rule

Do **not** delete or broadly rewrite `WAVE.md`, `STATE.md`, cards, Atlas, or
`jx3-wave-port` during this experiment.  They remain historical/current context
but must not block the pilot's paired diagnosis.  Only after a measured pilot
pass should the skill and templates be revised around generated ledgers and
boundary capsules.
