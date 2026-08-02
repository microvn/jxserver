# FND-RUNTIME-INITIALIZATION — initial evidence packet

Status: research only. This packet grants no source-write authority and makes
no candidate runtime-parity claim.

## Artifact identities

| Role | Artifact | SHA-256 |
|---|---|---|
| target DWARF/control-flow | `jx3_dwarf/SO3GameServerD` | `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a` |
| target runtime | `镜像端/extracted/root/SO3GameServer` | `3002bf4ad08d4c0fb9d4fc10f2d66b1f6cd8f70d398ff792136a3b7d5c416f11` |
| observed local candidate | `linux-build/SO3GameServer` | `f16725f5296ecb840612e85a240de43147a71c6dd1f9ece4b3a4b8b68c225259` |
| current Graph DB | `graphengine/evidence.sqlite` | `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba` |

The observed local candidate is not tied to a fresh compiler-input/build
manifest. Its relation to the active source tree is therefore
`OBSERVED_NOT_LINKED`, not an acceptance baseline.

## Native-Linux target runtime arm

The target binary was copied under a distinct oracle name to a native Linux
x86 host, verified again as SHA `3002bf4a…5c416f11`, and run temporarily in
the existing three-tier cluster. The active Wave 3 GameServer was copied to a
timestamped backup first and restored immediately after the probe; Center,
Gateway, and MySQL were not restarted.

The target log is SHA `402a5943e7e4a86962bc92f855d712a33fa79806181b0e156faa4722b882d015`.
It confirms:

- `19153 scripts searched`;
- `25627 AI loaded`;
- `6161 skill loaded`;
- `Load game settings ... [OK]`;
- Center connection, GameServer listener start, and server startup `[OK]`;
- map creation `[OK]`.

The three runtime DSO hashes and generated config hashes were captured in the
private wave evidence ledger. The probe also establishes an important fixture
rule: the VPS file previously named `SO3GameServer` had a different SHA and
must not be treated as a target oracle merely from its filename.

## Candidate P3 runtime arm

The existing P3 artifact was run with the same Linux host, topology, generated
configuration, DSO set, and data tree. It was exposed as
`SO3GameServer_w1_p3_candidate` and verified as the active executable.
Binary SHA is `1854b8aa5be8fe933d8b7e2fdcfb80a04667675b9e2588e7567459cfab441c10`;
the fresh log SHA is
`cc553868b494f8a0d1ad3e729213cedd92f15d8b5b80812e8b0f83e9f8f2a3cb`.

The candidate reaches the same foundation milestones and counts:

- `25627 AI loaded`;
- `6161 skill loaded`;
- `Load game settings ... [OK]`;
- Center connection, listener `3113`, and GameServer startup `[OK]`;
- map creation `[OK]`.

The candidate retains the known Wave 3 residual Lua fingerprints (`SetBOT` and
bounded `GetActivityMgrServer`). They are recorded, not filtered. This runtime
arm does not yet prove a fresh source/compiler closure; that manifest and the
opposite-model review remain required before accepting the feature card.

## Reopenable static evidence

```text
python3 graphengine/tools/query/evidence_query.py function 'KSO3World::Init' --limit 8
python3 graphengine/tools/query/graph_query.py callers 'KSO3World::Init' --limit 40
python3 graphengine/tools/query/graph_query.py callees 'KSO3World::Init' --limit 80
python3 graphengine/tools/query/evidence_query.py function 'KWorldSettings::Init' --limit 8
python3 graphengine/tools/query/evidence_query.py function 'KNpcTemplateList::Init' --limit 8
```

Confirmed target observations:

- `KSO3World::Init(IRecorderFactory*)` is at `0x0818f592` in the target
  DWARF binary. Symbol evidence is `29992`; target decompile evidence is
  `22427`; direct-call closure evidence is `30016`.
- `KSO3GameServer::Init@0x08050b0c` calls it at `0x08050d71`.
- The target decompile call inventory includes `KWorldSettings::Init`,
  `KScriptCenter::Init`, `InitAttributeFunctions`, `KAIManager::Init`,
  `KSkillManager::Init`, `KBuffManager::Init`, `KDropCenter::Init`, and other
  manager setup. The target direct call exists; exact ordered semantic
  equivalence with source/candidate is still unresolved.
- Candidate source roots are `src/SO3World/Src/KSO3World.cpp:71` and
  `src/SO3World/Src/KWorldSettings.cpp:8`.

## Required runtime evidence still absent

1. Complete card-scoped stock configuration/data/DSO manifest and working directory.
2. Equivalent candidate arm after a review-approved build.
3. Target DSO/provider evidence for every provider that becomes a candidate
   implementation decision.

## Documentation discrepancy

The older skill paths under `linux-build/.local/port-graph/docs/` no longer
exist. The current operational documents are under `graphengine/docs/`; their
build-history file names a V3 probe report that currently exists only in the
archive. This packet does not use the archived report as an active gate. Direct
Graph query results above remain valid only as static evidence.
