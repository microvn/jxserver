# KPlayerServer packet-validation drift (2026-07-28)

## Scope

This is a read-only comparison of the target v2.5.2 GameServerD and the local
candidate binary. It follows the live capture milestone where the TCP/security
connection remains open, but the client receives no scene/object-sync payload
after the 147-byte server frame.

Artifacts:

- Target: `jx3_dwarf/SO3GameServerD`, SHA-256 recorded in
  `artifact_hashes_20260728.sha256`.
- Candidate: `linux-build/SO3GameServer`, SHA-256 recorded in the same file.
- Raw disassemblies: sibling `target_*_20260728.txt` and
  `candidate_*_20260728.txt` files.

## Findings

### 1. Socket framing is structurally similar

`KG_AsyncSocketStream::Recv`, `KG_Packager::PackUp`, and
`KG_Package_RecvBuffer` both implement the same broad sequence:

1. accumulate a two-byte length;
2. reject lengths below the header minimum or above the per-stream limit;
3. receive the remaining bytes;
4. allocate/copy the payload;
5. pass the buffer through the package layer.

Target and candidate both use stream fields at `+0x2c/+0x30/+0x34/+0x38` and
the callback at `+0x48` in `KG_AsyncSocketStream::Recv`. The target has more
defensive errno/logging branches, but this comparison does **not** prove a
framing mismatch. No source patch is authorized from this observation alone.

### 2. The target and candidate packet validators are not the same contract

Target `KPlayerServer::CheckPackage`:

- address `0x08052640`;
- accepts only `uDataLen > 5`;
- protocol upper bound is `0xd0` (the `cmp ax, 0xd0; jbe` guard);
- protocol-size table base is indexed from `this + 0x1bc`;
- handler table is indexed from `this + 0x1bc` and a second table base around
  `+0xc`.

Candidate `KPlayerServer::CheckPackage`:

- address `0x081e2efa`;
- accepts only `uDataLen >= 6` (equivalent minimum);
- protocol upper bound is `0xdc`;
- protocol-size table base is indexed from `this + 0x1d0`;
- handler table uses a different base/stride arrangement around `+0xa`.

Both are called immediately by their respective `ProcessPackage` functions
after the stream `Recv` path and before protocol dispatch. Therefore this is a
real target/candidate contract boundary, not an unrelated helper difference.

### 3. ProcessPackage also has ABI/state-layout drift

The target and candidate `ProcessPackage(IKG_SocketStream*)` functions use
different `KPlayerServer`/connection-state offsets:

- target connection limit/state reads use offsets such as `+0x40`, player game
  status at `KPlayer + 0xd00`;
- candidate uses `+0x20` and player status at `KPlayer + 0xa4c`;
- the target calls `CheckPackage` at `0x0805432a`;
- the candidate calls it at `0x081e2bc4`.

This does not yet prove memory corruption or a rejected live packet. It proves
that the source candidate is not ABI/layout-equivalent to the target in this
packet-processing spine. A source edit must be based on target type/layout and
runtime evidence, not on copying the target constants into the candidate.

### 4. Constructor registration tables confirm the drift is systemic

The target and candidate `KPlayerServer` constructors initialise different
table regions, not just a single validator constant:

- target constructor `0x08086dbc` clears regions of `0x688` and `0x344` bytes
  and begins protocol registration around `this+0x74`, with size/handler table
  metadata around `this+0x6f8`;
- candidate constructor `0x081bab20` clears `0x6e8` and `0x374` bytes and begins
  the analogous registration around `this+0x70`, with metadata around
  `this+0x744`.

The raw constructor exports are saved as
`target_kplayerserver_ctor_20260728.txt` and
`candidate_kplayerserver_ctor_20260728.txt`. This makes a blind one-line
`CheckPackage` fix unsafe: the validator reads tables produced by a different
object layout and registration surface.

## What this explains and what it does not

The current capture proves the socket stays alive and reaches application
traffic. The validator drift is a strong candidate for why a post-selection
packet is not dispatched, but the encrypted capture alone does not identify its
decoded protocol ID or the exact failed predicate. The dossier therefore stays
`TARGET_STATIC_CONFIRMED` for the differences and `UNRESOLVED` for the live
failure predicate.

It does **not** justify:

- widening the candidate parser;
- changing `0xd0/0xdc` by hand;
- copying target object offsets into source;
- treating the 147-byte server frame as a confirmed scene packet.

## Next minimal probe

Run one hash-pinned candidate session with a temporary, bounded diagnostic at
the existing `ProcessPackage -> CheckPackage` boundary that records only:

- connection index;
- decoded `uDataLen`;
- decoded `byProtocolID` and frame;
- candidate `m_nProtocolSize[id]` and handler pointer;
- the first failed predicate, if any.

The diagnostic must be an isolated overlay and removed after capture. Compare
the same fields against a stock target arm if available. This single probe
distinguishes “candidate rejects the incoming post-selection packet” from
“candidate accepts it but the downstream handler/state gate stalls”.

## Status

`STATIC_TARGET_CONFIRMED / RUNTIME_PREDICATE_UNRESOLVED`.
