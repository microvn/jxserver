# Confirm-player-login relay envelope drift

Status: `TARGET_STATIC_CONFIRMED` + `RUNTIME_CORRELATED`; no source fix is
accepted by this dossier.

## Question

Why does the candidate accept the client handshake and then leave the client
on the loading screen without sending `C2S_APPLY_ENTER_SCENE`?

## Target authority

| Claim | Target evidence |
|---|---|
| `S2R_CONFIRM_PLAYER_LOGIN_REQUEST` is 10 bytes | `llvm-dwarfdump --name=S2R_CONFIRM_PLAYER_LOGIN_REQUEST -c jx3_dwarf/SO3GameServerD`; target `SO3GameServerD` SHA-256 `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c04954e51124b94a` |
| `dwPlayerID` is at offset `0x2` | same DWARF DIE |
| `dwIP` is at offset `0x6` | same DWARF DIE |
| Target sender allocates 10 bytes and writes both arguments | `SO3GameServerD:KRelayClient::DoConfirmPlayerLoginRequest(unsigned long,unsigned long) @ 0x080d3474`; disassembly saved in the session evidence ledger |
| Target handshake passes two values | `KPlayerServer::OnHandshakeRequest @ 0x0807a5dc` calls `DoConfirmPlayerLoginRequest` with `KPlayer+0x4` (player ID) and `KPlayer+0xa54` (`m_dwIP`) |
| `KPlayer+0xa54` is `m_dwIP` | `llvm-dwarfdump --name=KPlayer -c jx3_dwarf/SO3GameServerD`, member `m_dwIP`, offset `0xa54` |

Target disassembly facts:

```text
0x080d3496: KG_MemoryCreateBuffer(0xa)
0x080d3528: protocol id = 0xa
0x080d3530: arg1 -> packet + 0x2
0x080d3539: arg2 -> packet + 0x6
0x0807a741: call DoConfirmPlayerLoginRequest(player_id, m_dwIP)
```

## Candidate divergence

`linux-build/include/Include/Relay_GS_Protocol.h` currently declares:

```cpp
struct S2R_CONFIRM_PLAYER_LOGIN_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};
```

That is 6 bytes under the packed candidate headers. The candidate sender in
`linux-build/src/SO3World/Src/KRelayClient.cpp:3627` accepts only
`DoConfirmPlayerLoginRequest(DWORD dwPlayerID)`, allocates
`sizeof(S2R_CONFIRM_PLAYER_LOGIN_REQUEST)`, and writes only `dwPlayerID`.
`KPlayerServer::OnHandshakeRequest` calls it with one value.

The candidate also has `m_szClientIP` but no target-backed numeric `m_dwIP`
member at the target `KPlayer+0xa54` boundary. A source edit must therefore
first identify the existing candidate owner of the client address and prove
that its value has the target numeric representation; adding a zero or a
string-derived guess is not permitted.

## Runtime correlation

Active candidate diagnostic binary:

```text
SHA-256: d2ac0e73c42b5c51393e6d146cedec1c475949f144cc0672a0b180d49e1c8d33
BuildID: 23db311799f2bc429999dad7e7dedeb65717bc04
Process: /deploy/SO3GameServer_w1_fast_key (PID 781583)
```

Fresh client/GS capture:
`remote:/root/jx3/captures/w1-process-diag-20260728/entry.pcap`.

Fresh GS log:
`remote:/root/jx3/镜像端/extracted/root/logs/SO3GameServer/2026_07_27/SO3GameServer_2026_07_27_17_51_07.log`.

Observed sequence:

```text
client -> GS: C2S_HANDSHAKE_REQUEST (decoded proto=1, payload len=26, expected=26)
GS log: Bind 90175com:(ThatTu, 6) to connection(0)
GS -> client: one 147-byte encrypted frame
client -> GS: C2S_PING_SIGNAL (decoded proto=6, payload len=10, expected=10), repeated
client -> GS: no C2S_APPLY_ENTER_SCENE (proto=3)
GS log: no OnConfirmPlayerLoginRespond / permit / base-info transition
```

This does not prove the Center's internal parser rejection by itself, because
the internal packet capture was armed after this role attempt. It does prove
that the candidate reaches the handshake handler but never reaches the
post-permit client entry path. The missing target `dwIP` field is the first
target-backed boundary that explains that exact stop and must be tested before
changing client/GS transport or `CheckPackage` constants.

## Required bounded patch/review

1. Identify the candidate numeric client-IP owner and pin its value at the
   handshake callsite; if unavailable, add a separate unresolved evidence item.
2. Extend only the confirm-player-login request envelope and sender signature
   to target size/offsets.
3. Build from the complete canonical baseline; no other protocol or parser
   changes in the same overlay.
4. Opposite-model review must check the raw diff against the target DWARF and
   callsite disassembly.
5. Capture GS↔Center port 5003 and client↔GS together. Acceptance requires a
   target-sized confirm request, a permit response, and the first client
   `C2S_APPLY_ENTER_SCENE`; this dossier does not claim that patch or acceptance.

## Explicit non-claims

- `CheckPackage` upper bound (`0xd0` vs candidate `0xdc`) is not implicated by
  this runtime: handshake and ping both pass its candidate validator.
- `KSG_DecodeEncode` is not implicated by this observation; its previous
  off-by-one hypothesis was rejected by the cross-model review.
- This dossier does not claim that adding `dwIP` alone completes enter-world;
  it identifies the next falsifiable relay boundary.
