# Journey Contract Query: blind target-static prototype

This frozen prototype projects only current target-static GraphEngine evidence.
It is a research aid for selecting the next narrow target closure, not a source
patch plan or runtime acceptance result.

The frozen seed is `r2s_handshake_respond`, selected because the roadmap places
Relay role-envelope work ahead of enter-world and the immutable GraphEngine DB
contains a target-backed protocol/handler/payload record for it.

`journey-contract.target-static.json` contains, in order:

1. seed;
2. target protocol/dispatch;
3. target handler anchor;
4. target payload type and only DWARF-backed fields;
5. bounded direct target call observations; and
6. the required candidate outbound/send stage as `UNRESOLVED`.

Every claimed edge carries target origin, authority, status, artifact SHA,
binary SHA, address or DIE where available, and evidence ID. The tool excludes
source, candidate, and runtime origins entirely. It never infers payload order,
candidate behavior, or runtime behavior.

## Reproduce

Run the command in `COMMAND.txt`, then validate with:

```bash
python3 -m json.tool journey-contract.target-static.json >/dev/null
sha256sum -c SHA256SUMS
```

The DB is immutable input for this artifact. A different DB hash requires a new
frozen output rather than overwriting the interpretation of this one.

## V2 changelog

`journey-contract-query/v2` adds bounded `outbound_contract_expansion` and one
optional direct second hop. A direct handler callee is linked only through a
materialized target enum/protocol or payload-DWARF bridge; spelling, apparent
response direction, and call order are not evidence. The V2 frozen seed is
`r2s_player_login_request`. Its output records the observed generic gap:
target direct calls exist, but no materialized target contract link proves an
outbound wire contract or payload flow for them, so the stage is `UNRESOLVED`.

See `COMMAND.txt`, `PROVENANCE.md`, and `journey-contract-v2.target-static.json`.

## V3 changelog

V3 reopens only bounded direct callees from the V2 closure. It emits an
`outbound_serializer_lane` only when the same target function has a target
decompile buffer allocation/header literal and direct `Send` callsite, while
the literal resolves to a target enum and an equal-size target DWARF payload
type. The result remains `TARGET_STATIC_PARTIAL`: target static evidence does
not prove field writes/data flow into the sent buffer or runtime delivery.

## V4 changelog

V4 adds a generic multi-binary protocol projection for the three target
processes. Each binary retains its own target SHA, artifact, enum/type/DIE, and
dispatch evidence. A repeated enum/type identity becomes only a
`TARGET_STATIC_PARTIAL` bridge; it is not a process route, packet observation,
or runtime claim.
