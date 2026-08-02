# Role-state V2 audit — evidence-first patch gate

Status: target audit complete; source patch scope is limited to the active
`rbtStateInfo` failure. This file records the evidence used before editing.

## Target identity

- Binary: `jx3_dwarf/SO3GameServerD`
- Binary SHA-256: `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`
- Graph DB target decompile evidence: `evidence_id=30036`
- Graph target type evidence: `evidence_id=30018`
- Graph target call-closure evidence: `evidence_id=30016`
- Deep decompile export: `raw/audit-20260728/target-role-load-deep.jsonl`

## Three-source cross-check

### DWARF

`nm | c++filt` confirms the target signatures and addresses:

```text
KPlayer::LoadStateInfo(unsigned char*, unsigned int, int) @ 0x0839b112
KPlayer::LoadStateInfo_V0(unsigned char*, unsigned int)    @ 0x0839ab6a
KPlayer::LoadStateInfo_V1(unsigned char*, unsigned int)    @ 0x0839a5b0
KPlayer::LoadStateInfo_V2(unsigned char*, unsigned int)    @ 0x0839a01c
KPlayer::PartialLoadExtData()                              @ 0x0839b62e
KPlayer::OnExtDataLoadFinish()                             @ 0x0839fb50
```

DWARF defines the packed target payloads:

| Version | Target type | Size | DIE |
|---:|---|---:|---|
| 0 | `KROLE_STATE_INFO_V0` | `0x75` | target DWARF at declaration line 258 |
| 1 | `KROLE_STATE_INFO_V1` | `0x80` | target DWARF at declaration line 305 |
| 2 | `KROLE_STATE_INFO_V2` | `0x80` | `0x059e5856`, declaration line 352 |

The target block header is 12 bytes and has `nType@0`, `dwVer@4`, and
`dwLen@8`. `ROLE_DATA_BLOCK_TYPE.rbtStateInfo = 5` and `rbtTotal = 49`.

The V2 payload offsets are target facts, not source-order guesses. The key
offsets are:

```text
move state 0x00, track 0x01, frame 0x03, fly node 0x07, city 0x09
experience 0x0b, life 0x0f, mana 0x13
train-today 0x17, train-current 0x1b, train-used 0x1f
revive-frame 0x25, revive-time 0x27, revive-count 0x2b, killer 0x2c
kill-point 0x30, ban-time 0x32, max-level 0x36, hide-hat 0x3a
killed-count 0x3e, killed-reset-time 0x3f, camp-flag 0x43
last-train-time 0x44, title/rank fields 0x48..0x5a
sprint/exterior fields 0x5e..0x66
daily counters 0x67..0x74, hunter/prison/fake-name 0x75..0x78
reserved tail 0x79..0x7f
```

### GraphEngine

The evidence-first queries returned:

- `function KPlayer::LoadStateInfo`: target address, source/candidate
  signature drift, and target call closure; no semantic equivalence claim.
- `function KPlayer::LoadStateInfo_V2`: target address `0x0839a01c`, target
  decompile document present, and target calls retained with evidence IDs.
- `function KPlayer::PartialLoadExtData`: target call at `0x0839bc56` to the
  4-argument dispatcher is `TARGET_STATIC_CONFIRMED`.
- `type KROLE_STATE_INFO_V2`: target DWARF fields/offsets are present; source
  and runtime bridges remain unresolved, as expected.

The Graph correctly does not claim source/candidate semantic parity. Its
`RAW_SUSPECT_SELF_CALL` rows were excluded from the control-flow conclusion.

### Target decompile

The target `PartialLoadExtData` decompile at `0x0839b62e` proves:

```text
read 12-byte KRoleBlockHeader
→ validate dwLen against remaining bytes
→ for type 5 call LoadStateInfo(payload, dwLen, dwVer)
→ reject the block if the call returns false
→ only then advance/ack the processed section
```

The dispatcher at `0x0839b112` proves:

```text
version 0 → LoadStateInfo_V0; require 0x75 bytes
version 1 → LoadStateInfo_V1; require 0x80 bytes
version 2 → LoadStateInfo_V2; require 0x80 bytes
other     → fail/assert path
```

The V2 decompile at `0x0839a01c` consumes exactly `0x80` bytes and rejects
non-zero trailing bytes. It also confirms that V2 is not the legacy struct:
it has no legacy `byPKState` field at the old offset and includes the expanded
title/daily-counter tail.

## Candidate divergence and live correlation

Candidate currently has:

```text
LoadStateInfo(BYTE*, size_t)
LoadExtRoleData → LoadStateInfo(payload, dwLen)
KROLE_STATE_INFO legacy payload: 117 bytes
```

The live candidate log reported `type=5`, `version=2`, `len=128`, followed by
`uLeftSize == 0` failure in the legacy parser. This is exactly the target/
candidate contract mismatch above. The failure occurs before ext-data
completion and before `DoSyncRoleDataOver`/ready.

## Patch authorization and limits

Authorized next patch:

1. Add the target-backed version argument and dispatcher.
2. Add the packed V2 payload definition/reader for the fields that have a
   candidate destination; consume and validate the complete 128-byte payload.
3. Pass `dwVer` from `KRoleBlockHeader`.
4. Preserve the existing section-ack and ready ordering.

Not authorized by this audit:

- renumbering role-block enums;
- rewriting all 49 role blocks;
- treating source 2010 declaration order as V2 truth;
- skipping unknown blocks by silently changing `dwLen`;
- changing transport, client code, or `libcommon.a`.

Acceptance for this bounded patch is removal of the state-block parse error and
observation of the next target-backed completion milestone. It is not yet a
claim that every V2 role field or every role block is fully ported.
