# JX3 Port Evidence Layers

This directory is the measurement and evidence system for the JX3 GameServer
port from the 2010 source baseline to the v2.5.2 target server.

It answers a deliberately narrow question:

> What does the target prove, what does the current rebuilt server provide, and
> which feature contracts still need evidence before a port can be accepted?

It is **not** a replacement for the reverse-engineering graph, a source tree,
or a test suite. It is the public, reproducible layer between them: small tools
extract facts from named artifacts, retain their evidence next to the tool that
produced it, and make gaps visible before implementation begins.

## Read this first

The port has four distinct truth domains. They must not be collapsed into one
unqualified result:

| Domain | What it establishes | Typical authority |
|---|---|---|
| Target contract | What v2.5.2 actually contains or does | DWARF, target binary, decompile, packet capture |
| Source baseline | What the 2010 implementation originally does | source file and revision |
| Candidate implementation | What the rebuilt server currently contains | candidate source, build artifact, build provenance |
| Runtime acceptance | What the target or rebuilt system does in a bounded scenario | normalized stock/built trace, log, packet observation |

The layers below mainly compare the first three domains. A green measurement is
therefore **evidence of one property**, not a claim that a gameplay feature is
complete. Runtime acceptance still belongs to a Feature Card and its bounded
scenario.

## Directory layout

```text
layers/
├── README.md                         this document
├── run_all.sh                        runs L1-L7 and writes COVERAGE.md
├── gen_coverage_report.py            aggregates existing layer evidence
├── inventory/                        target method inventory, used to build PORT_MAP.csv
│   └── evidence/                     normalized symbol-diff inputs
├── L1-symbol-coverage/               target symbol surface versus rebuilt binary
│   └── evidence/
├── L2-lua-bindings/                  Lua-visible C++ surface and Lua enum table evidence
│   └── evidence/
├── L3-data-config/                   data/config reader and boot-log observations
│   └── evidence/
├── L4-enum-contracts/                target enum ordinal contracts
│   └── evidence/
├── L5-serialized-layouts/            packet and persistence layout contracts
│   └── evidence/
├── L6-lua-triggers/                  C++-to-Lua callback fire-site coverage
│   └── evidence/
└── L7-logic-drift/                   same-name logic drift candidates and acceptance ledger
    └── evidence/
```

Each layer owns both its producer and its evidence. Do not move all generated
files into a generic output directory: separating an output from the tool,
question, authority, and input configuration that created it makes the result
hard to audit.

`inventory/` is intentionally separate from L1. It describes the target method
surface used to generate `../PORT_MAP.csv`. L1 asks a different question: whether
the current rebuilt binary exports the relevant target symbol surface.

## Evidence rules

Every layer must follow these rules.

1. **One question per layer.** A layer may have supporting scripts, but it must
   answer one stable, reviewable question.
2. **Evidence is origin-preserving.** A target observation, source observation,
   candidate observation, and runtime observation remain distinguishable.
3. **Authority is explicit.** Source 2010 may explain an implementation, but it
   cannot override the v2.5.2 binary/DWARF contract.
4. **Unknown is a valid result.** A missing candidate symbol, an unmeasured
   packet field, or no runtime capture must be reported as unresolved rather
   than inferred from a similarly named source method.
5. **Generated evidence is retained.** A newer capture supersedes an older
   capture; it does not make the older capture disappear without provenance.
6. **Public-safe only.** No host address, local absolute path, account data,
   secret, or unredacted runtime log belongs here. Pass environment-specific
   locations at execution time.
7. **A layer is not a port ticket.** A gap becomes work only after it is placed
   in a Feature Card with scope, dependency closure, target evidence, and an
   acceptance scenario.

When adding a new layer, document its question, authority, inputs, output
schema, known false-positive modes, and how it affects Feature Card decisions.

## Layer catalogue

### Inventory — target method surface

**Question:** Which class methods are visible in the target surface, which are
present in the 2010 source, and which rows require further mapping?

**Authority:** Target DWARF/binary symbol evidence for the target side; source
files for the baseline side.

**Main tools:**

- `extract_symbols.sh` — extracts normalized target/source symbol inputs.
- `classify_symbol_drift.sh` — categorizes symbol-level differences.
- `diff_bin_vs_source.py` — quick binary/source method-surface comparison.
- `generate_method_inventory.py` — generates `../PORT_MAP.csv` from normalized
  evidence.

**Evidence:** `inventory/evidence/` retains normalized symbol-diff inputs such
as `missing_full.tsv` and `per_class_full.tsv`.

**Interpretation:** A target-only method is a research lead, not an instruction
to copy a file or invent a stub. It may be a new feature, an inline method, a
renamed method, a compiler artifact, or a method supplied by a linked library.

### L1 — Symbol coverage

**Question:** Which class-scoped v2.5.2 symbols are absent from the current
rebuilt server binary?

**Authority:** v2.5.2 DWARF symbol surface compared with symbols from the rebuilt
candidate binary.

**Main tools:** `cpp_coverage.sh`, `cpp_rollup.py`, and `systems.py`.

**Typical evidence:** `cpp_missing_members.txt`, `cpp_missing_by_class.tsv`, and
`cpp_by_system.tsv` under `L1-symbol-coverage/evidence/`.

**What it catches:** Missing methods, missing class surfaces, and systems with a
large target/candidate gap.

**What it cannot establish:** Correct control flow, field layout, packet shape,
or runtime behavior. A symbol can exist and still implement stale 2010 logic.

### L2 — Lua bindings

**Question:** Which Lua-facing C++ bindings exist in the target but not in the
rebuilt server, and which missing bindings are demanded by deployed scripts?

**Authority:** Target binary symbols for the provided binding surface; deployed
script calls only rank the impact of a gap. They do not prove registration shape
or argument semantics by themselves.

**Main tools:** `lua_coverage.sh` and `extract_lua_attr.py`.

**Typical evidence:** `lua_gap_ranked.tsv` and the target
`_lua_attr.tsv` table under `L2-lua-bindings/evidence/`.

**What it catches:** A script that calls a Lua binding the rebuilt engine does
not expose, including high-demand gaps likely to fail as `nil` at runtime.

**What it cannot establish:** Object ownership, argument coercion, return values,
metatable shape, or call ordering. Those require decompile and, where relevant,
runtime evidence.

### L3 — Data and configuration readers

**Question:** Which configuration/data-reader mismatches are observed at boot,
and which named config keys differ between target and source reader logic?

**Authority:** Runtime boot log for observed failures; target decompile plus the
data header for reader-key/section mapping.

**Main tool:** `diff_loaddata.py`. `run_all.sh` also normalizes the latest boot
log into `data_gaps.tsv`.

**Typical evidence:** `data_gaps.tsv` in `L3-data-config/evidence/`.

**What it catches:** Missing files, invalid table reads, and high-frequency
boot-time config gaps.

**What it cannot establish:** On-demand tables that are never reached during the
captured boot, nor whether a tolerated error later becomes gameplay-visible.

### L4 — Enum contracts

**Question:** Which target enum values and members differ from the 2010 source?

**Authority:** v2.5.2 DWARF enum definitions and target enum-convertor binary
evidence. The source enum is comparison data, not the target contract.

**Main tools:** `enum_audit.py` and `extract_enum_maps.py`.

**Typical evidence:** `enum_drift.tsv`, target enum cache, and extracted
`_enum_*.tsv` maps in `L4-enum-contracts/evidence/`.

**What it catches:** Ordinal drift that symbol coverage misses, including cases
that can silently break AI action lookup, Lua constants, protocol IDs, or table
conversion.

**What it cannot establish:** The payload layout or semantic meaning of a
protocol just because its enum ID matches.

### L5 — Serialized layouts

**Question:** Which packet and persistence structures have a target byte-size
contract, and which candidate structures still require verification?

**Authority:** v2.5.2 DWARF byte sizes for serialized packet and DB-role types.

**Main tools:** `struct_audit.py` and `s2c_sizeof_probe.cpp`.

**Typical evidence:** `struct_sizes.tsv` and target layout cache in
`L5-serialized-layouts/evidence/`.

**What it catches:** Candidate layouts that cannot safely be assumed compatible
with target packet or persistence contracts.

**What it cannot establish:** Field order, field meaning, conditional encoding,
or a complete protocol decoder. A size match alone is never sufficient for a
network or role-data acceptance claim.

### L6 — Lua trigger paths

**Question:** Which callback names are fired by the target C++ engine but have
no corresponding fire-site in the candidate source, while content scripts define
the callback?

**Authority:** Target binary string evidence and candidate source fire-site
evidence; script definitions provide impact context.

**Main tool:** `trigger_audit.sh`.

**Typical evidence:** `trigger_gap.tsv` in `L6-lua-triggers/evidence/`.

**What it catches:** A pure-Lua feature whose script exists but is never entered
because the C++ event bridge does not fire it.

**What it cannot establish:** That a matching callback name is fired at the
correct time, with the correct arguments, or through the correct Lua state.

### L7 — Logic drift

**Question:** Among same-named functions present in both target and candidate,
which ones are likely to have different logic and therefore need targeted
reverse-engineering?

**Authority:** Target and candidate disassembly/decompile observations. The
callee-set comparison is a prioritization signal, not a behavioral proof.

**Main tools:** `callee_audit.py`, `callee_compare.py`, and `bsim_lite.py`.

**Typical evidence:** `callee_drift.tsv`, callee maps, optional body-similarity
reports, and `logic_verified.txt` in `L7-logic-drift/evidence/`.

**Important limitation:** Compiler version, inlining, thunks, and constructors
can create false positives. Conversely, a matching callee set can hide a changed
condition, constant, field offset, or packet field. L7 ranks investigation; it
does not accept code.

`logic_verified.txt` is different from generated evidence: it is an acceptance
ledger. Add an entry only after the function has a target dossier and the
Feature Card records the applicable oracle/decompile/runtime check.

## Running the measurement harness

Run from the `linux-build` repository root. The repository deliberately has no
embedded remote host or remote path; provide them per invocation.

```bash
HOST=<ssh-target> \
OURS_REMOTE=<path-to-rebuilt-server-binary-on-target> \
SCRIPTS=<path-to-target-script-directory-on-target> \
LOGDIR=<path-to-gameserver-log-directory-on-target> \
bash port_plan/layers/run_all.sh --force
```

`--force` refreshes the cached target DWARF extraction used by the enum, layout,
and logic layers. Omit it when intentionally reusing a known capture.

The harness writes each result to the owning layer's `evidence/` directory, then
creates `COVERAGE.md` at this directory's root. `COVERAGE.md` is a navigation
dashboard; it is not an acceptance artifact.

To run one layer, invoke its producer directly and provide only the variables
that tool requires. For example, a layer that compares a rebuilt binary needs
`HOST` and `OURS_REMOTE`; a Lua-demand layer also needs `SCRIPTS`.

## How a Feature Card should use layers

Before changing source for a feature:

1. Read `inventory/` and `../PORT_MAP.csv` to identify likely target members.
2. Check L1 for missing target surface.
3. Check L2 and L6 when Lua is on the feature path.
4. Check L4 for enum IDs and constants used by the feature.
5. Check L5 for packets, role blobs, or serialized structures.
6. Check L3 for tables/configuration read by the feature.
7. Use L7 to prioritize same-name functions that need a target dossier.
8. Record the exact evidence files, target artifact identity, unresolved facts,
   and bounded runtime acceptance scenario in the Feature Card.

Do not implement every result from every layer. The Feature Card selects the
finite evidence closure required for one feature. Unrelated gaps remain visible
for later work without being silently mixed into the patch.

## Common misreadings

| Incorrect conclusion | Correct interpretation |
|---|---|
| “L1 is green, so the feature is ported.” | The expected symbols exist; semantics and contracts may still drift. |
| “L4 enum IDs match, so the protocol is correct.” | The numeric route may match while payload layout or handler behavior differs. |
| “The boot log is clean, so all data loads are correct.” | Only the captured boot path is covered; on-demand content remains unobserved. |
| “L7 found a drift candidate, so rewrite the function.” | Investigate with target decompile/dossier first; it may be compiler noise. |
| “The target has no symbol, so source code is unnecessary.” | Source remains the implementation baseline; absence may indicate inlining, rename, or linked code. |
| “The dashboard percentage is an acceptance result.” | It is a triage metric. Acceptance requires feature-specific target and runtime evidence. |

## Maintaining this directory

- Keep layer names stable; they are referenced by Feature Cards and external
  progress reports.
- Prefer additive evidence captures over overwriting unexplained older captures.
- State the target artifact identity and extraction date in any generated
  manifest added to an `evidence/` directory.
- Keep large private reverse-engineering corpora, full decompiles, runtime logs,
  database dumps, and graph databases outside this public plan. Link to their
  evidence identifiers in private documentation instead.
- If an existing layer is insufficient, improve its extraction contract rather
  than hard-coding an answer for one Wave or one feature.

The intended outcome is a compact, auditable system: enough structure to make a
port decision reproducible, without turning every symbol, string, or heuristic
into an unreviewable graph of its own.
