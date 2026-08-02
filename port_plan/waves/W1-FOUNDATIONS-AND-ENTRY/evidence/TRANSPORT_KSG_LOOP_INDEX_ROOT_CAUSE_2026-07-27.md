# FND transport: rejected KSG decode-loop hypothesis

Status: `NOT_LOCALIZED / CODEC_CLEARED_BY_CLAUDE_REVIEW`

This record is retained as a negative result. Its original off-by-one codec
claim was independently disproved; no source, archive, build, deployment, or
runtime mutation followed from it.

## Independent review result

Claude review session `a3303275` opened the target DWARF executable and the
local candidate archive directly.

| Item | Confirmed result |
|---|---|
| Target | `KSG_DecodeEncode @ 0x083cc0dc` decrements `word_count` before its body, so its first lookup is `key + word_count - 1`. |
| Candidate | `KSG_DecodeEncode @ 0x082b1070` has the same effective order. |
| Archive owner | `libs/libcommon.a(KSG_EncodeDecode.o)`, not `KG_Socket.o`. |
| Algorithm | table, divisor `5679`, key update, tail handling, and `KSG_DecodeBuf` dispatcher agree. |
| Independent simulation | 400 randomized size/key cases: no buffer or returned-key mismatch. |

The original comparison saw the candidate's pre-loop decrement but missed the
target's `decl 0x8(%ebp)` in the target loop test. The target and candidate are
bit-exact for this codec path.

## Unsafe patch explicitly rejected

The proposed byte replacement at candidate function offset `+0x1e`,
`83 eb 01 83 fb ff -> 85 db 74 3c 90 90`, must never be applied. For non-empty
input it would retain the existing loop termination while adding one more loop
iteration, XORing and writing one dword past the intended word range. It is an
unnecessary out-of-bounds write on client-controlled input.

## Evidence limitation and next direction

The temporary trace executable, pcap, and replay harness cited by the original
claim are not retained in this Wave package, so the historical decoded-header
replay cannot be reproduced. It cannot support a transport conclusion.

The next investigation is outside `KSG_DecodeEncode`:

1. Capture and retain raw first-frame traffic plus the replay harness.
2. Compare target/candidate `KG_AsyncSocketStream::Recv` framing, input slice,
   and number of decode calls (target object `0x58`, candidate `0x4c`).
3. Compare receive callsites and selection of `KSG_DecodeBuf`,
   `KSG_DecodeDynamic`, and `KSG_DecodeBufSimple`.
4. Only patch after a new target-backed, reproducible divergence is found.
