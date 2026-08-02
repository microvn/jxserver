# S2C_SYNC_PLAYER_BASE_INFO target drift

## Finding

The latest capture showed the candidate sending a 153-byte `S2C_SYNC_PLAYER_BASE_INFO`
packet immediately after the Center permit response. Target DWARF and target
`KPlayerServer::DoSyncPlayerBaseInfo` prove that the v2.5 packet is 0xa1 (161)
bytes, not 0x99 (153).

## Target evidence

- Target: `jx3_dwarf/SO3GameServerD`
- Target SHA: `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`
- `S2C_SYNC_PLAYER_BASE_INFO` DWARF size: `0xa1`
- Target `DoSyncPlayerBaseInfo`: `0x0807a11e`
- Target sends `0xa1` at `0x0807a2c7`
- Target fields missing from the old candidate declaration: `nChargeMode`,
  `byAccountMaxLevel`, `byIsBankPasswordVerified`, `byMibaoMode`,
  `nServerTimezone`, `nServerDaylight`, `bySelectKungfuIndex`.
- Target `PLAYER_EQUIP_REPRESENT::perRepresentCount` is 33. The canonical
  source now pins the six target additions (`perWeaponColor`, the four
  `perBigSword*` entries, and `perFaceExtend`) rather than assuming the remote
  tree already had them.

## Runtime evidence

- Capture: `/root/jx3/captures/w1-confirm-observe-20260727T182437/entry.pcap`
- Candidate server→client packet at `2026-07-27 18:25:57.201424`: TCP payload
  length `153` (`0x99`).
- Immediately before it, GS sent `protocol 10 / size 10` to Center and Center
  returned the permit response; this localizes the next drift at the GS→client
  base-info boundary, not at `DoConfirmPlayerLoginRequest`.
- After the 153-byte packet, the client emitted only 12-byte heartbeat packets;
  no `C2S_APPLY_ENTER_SCENE` was observed.

## Candidate change

The isolated candidate declaration now includes the seven target tail fields and
the producer initializes them to zero until their runtime owners are separately
proven. This is a wire-size correction, not a claim that all field semantics are
complete.

The first deploy claim was stale: the active VPS header had reverted to the
pre-tail declaration and disassembly still passed `0x91` (145 body bytes, 153
on the wire). The remote worktree was reconciled from the canonical header,
rebuilt natively, and the active `SO3GameServer_w1_fast_key` is now SHA
`e607ee5414d835188f2083c797162e808b71383b9ee0aa66d1410865fcd24ddd`.
Its `DoSyncPlayerBaseInfo` disassembly passes `0xa1` to `Send`.

## Next gate

Run one selected-role client attempt against this hash-pinned binary and capture
the first packet after permit. Acceptance requires a 161-byte base-info packet
and a subsequent client `C2S_APPLY_ENTER_SCENE`; otherwise continue at the next
observed boundary without interpreting the UI text as a protocol result.
