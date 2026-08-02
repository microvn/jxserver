# Provenance

- Frozen UTC: `2026-07-27T17:17:05Z`
- Query mode: `blind-target-static`
- Graph DB: `linux-build/graphengine/evidence.sqlite`
- DB SHA-256: `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba`
- Target origin filter: `target%`
- Explicitly excluded origins: `source-2010`, `candidate`, `runtime`
- Frozen command: `COMMAND.txt`
- JSON validation: `python3 -m json.tool` passed before freeze.

The selected target protocol observation is evidence ID `30001` with status
`TARGET_STATIC_CONFIRMED`. It names the target handler anchor and carries the
payload type/DIE/field evidence. Direct closure edges are separate target
control-flow observations and remain bounded by the recorded command.

This package does not claim that the candidate emits any protocol, that a send
succeeds, or that the target behavior occurs at runtime.

## V2 freeze

- Schema: `journey-contract-query/v2`
- Seed: `r2s_player_login_request`
- Bounds: 16 direct calls, 16 outbound candidates, 4 second-hop calls each.
- Output: `journey-contract-v2.target-static.json`
- Tool SHA-256: `5e02624b3188abe41d9304bdfc850f3b30794a10424535cff05b703d33c3d3b9`
- Output SHA-256: `23a5d4872da14840fd7be857a9875530b749fcb487e605727f08b2abd34f2938`

The target-only result is `TARGET_STATIC_PARTIAL`. The outbound expansion has
no materialized enum/protocol or payload-DWARF linkage for any inspected direct
callee; its summary is explicitly `UNRESOLVED`. This is a GraphEngine evidence
gap, not a claim about target wire behavior or the candidate.

## V3 freeze

- Schema: `journey-contract-query/v3`
- Seed: `r2s_player_login_request`
- Output: `journey-contract-v3.target-static.json`
- Tool SHA-256: `15faa789004780bc60586f99f3ba5ac8bda02c684361f527c09ad86e68f54f89`
- Output SHA-256: `c1dbd49e06229fb11f7a98ac375ef078e3ad55f76e127aafd71d4436f6d37e11`

V3 uses only the target DB's decompile, call, enum, and DWARF observations.
Its serializer result is static-partial and carries the exact artifacts,
callsite, enum/DIE, type DIE, and payload-field DIES. It does not claim a
packet-field data flow, send success, or runtime behavior.

## V4 freeze

- Schema: `journey-contract-query/v4`
- Input: `r2s_player_login_request`
- Output: `journey-contract-v4.target-static.json`
- Tool SHA-256: `f6aed3c9e05b5fa75637d65d6f3d3b22a3504e1d6690e78f32dac2aeddbe82b6`
- Output SHA-256: `d53548d3688e32f24334b4c8970e4a4868b8c13264bdf0dc974a30be23ce2a26`

V4 reports separate target observations for GameServer, GameCenter, and
Gateway. The frozen input has enum/type identity in two binaries and therefore
one `TARGET_STATIC_PARTIAL` bridge; only GameServer has a materialized target
dispatch/handler observation. Gateway remains `UNRESOLVED` for this input.
