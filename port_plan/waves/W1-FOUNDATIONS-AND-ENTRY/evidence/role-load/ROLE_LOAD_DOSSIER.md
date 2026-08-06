# FND-ROLE-LOAD-SPINE — target-backed static dossier

Status: research evidence only. This dossier creates no source-write authority and
does not claim runtime parity.

## Scope

The bounded role-load spine is:

```text
R2S_PLAYER_LOGIN_REQUEST
  -> KRelayClient::OnPlayerLoginRequest
  -> KPlayer::LoadBaseInfo
  -> role-data blocks
  -> KPlayer::LoadStateInfo(versioned)
  -> first initial-sync consumers
```

The Relay envelope and its live ordering remain owned by
`FND-RELAY-ROLE-ENVELOPE`. This dossier only pins what the GameServer must
decode after that envelope reaches it.

## Target authority and reproducibility

| Fact | Target evidence |
|---|---|
| Target executable identity | `jx3_dwarf/SO3GameServerD`, SHA-256 `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a` |
| Target `KPlayer::LoadStateInfo(BYTE*, unsigned int, int)` | DWARF symbol address `0x0839b112` |
| Target `KPlayer::LoadBaseInfo(KRoleBaseInfo*)` | DWARF symbol address `0x0839dce2` |
| Target `KRoleBaseInfo` | DWARF DIE `0x00a7fdfd`, size `0xa2` |
| Target `ROLE_DATA_BLOCK_TYPE` | DWARF DIE `0x059e4ec8`, `rbtTotal = 49` |
| Target decompile export | `raw/target-role-load-pyghidra.jsonl`, SHA-256 `2a71efcceb0f9cd95ea04c04fbc93a3ad8d446cbc2d4d73c8e53e8a6465e3db7` |
| Deep target closure export | `raw/audit-20260728/target-role-load-deep.jsonl`, SHA-256 `5d3aca03b9e4a8735c2a553e2e590f32be0c34e0e53423e19f8d76fd1ad11949` |
| Graph snapshot | `compare-engine/evidence.sqlite`, SHA-256 `25e726bf747b3e6ecfe7c045bd7c1a1e1c22610a4ab917cbc37a93f4d45449ba` |

Reproduce the static extraction without relying on the graph's reconciliation
layer:

```bash
DD=/opt/homebrew/opt/llvm/bin/llvm-dwarfdump
"$DD" --name='KRoleBaseInfo' -c jx3_dwarf/SO3GameServerD
"$DD" --name='ROLE_DATA_BLOCK_TYPE' -c jx3_dwarf/SO3GameServerD
nm jx3_dwarf/SO3GameServerD | c++filt -n | rg 'KPlayer::(LoadStateInfo|LoadBaseInfo)\\('
python3 compare-engine/tools/extract/pyghidra_export.py jx3_dwarf/SO3GameServerD \
  --out <out>.jsonl --no-analysis --with-xrefs \
  --decompile LoadBaseInfo --decompile LoadStateInfo
```

## Confirmed target facts

### 1. State blocks are versioned, not one fixed source-era layout

`KPlayer::LoadStateInfo` at `0x0839b112` accepts `(BYTE*, unsigned int,
int version)` and dispatches only versions `0`, `1`, and `2` to
`LoadStateInfo_V0`, `_V1`, or `_V2`. Any other version returns failure.

The target variants have distinct fixed consumed lengths:

| Target parser | Minimum/consumed state payload |
|---|---:|
| `LoadStateInfo_V0` | `0x75` bytes |
| `LoadStateInfo_V1` | `0x80` bytes |
| `LoadStateInfo_V2` | `0x80` bytes |

This is direct target decompile evidence. It proves the version argument is a
required contract dimension. The deep `PartialLoadExtData @ 0x0839b62e`
decompile also proves that the version is read from the second DWORD of each
12-byte block header and passed into this dispatcher. It does **not** yet prove
which version a particular stored role blob uses.

### 2. Base-info wire layout is target-defined

Target DWARF defines `KRoleBaseInfo` as 162 bytes. Relevant offsets are:

```text
cRoleType                         0x04
CurrentPos                        0x05
LastEntry                         0x1e
byLevel / byCamp / byForceID      0x37 / 0x38 / 0x39
wRepresentId[33]                 0x3a
nLastSaveTime / nLastLoginTime    0x7c / 0x80
nTotalGameTime / nCreateTime      0x84 / 0x88
mentor/corps tail                 0x8c .. 0xa1
```

`KPlayer::LoadBaseInfo` at `0x0839dce2` calls `KSO3World::GetScene` with
the `CurrentPos` map/copy fields at base-info offsets `+0x09` and `+0x0d`.
The target's `KROLE_POSITION_DB` contains `nCenterIndex` at its offset 0,
therefore `dwMapID` follows at role-base offset `0x09`. Source field order
cannot be used as the target ABI contract.

### 3. Role-block namespace is larger than the source dispatch

Target DWARF defines `rbtTotal = 49` (values 0 through 48). The current
candidate header ends at `rbtCurrencyData` then `rbtTotal`; it does not cover
the target-only range 28 through 48. Current `KPlayer::LoadExtRoleData` logs
and skips its default/unhandled block; its save path emits only known blocks.

Therefore unknown-block byte preservation is a required implementation item
for a future role-load patch, not an optional test enhancement.

## Current candidate observations (not target truth)

| Candidate observation | Location | Interpretation |
|---|---|---|
| `OnPlayerLoginRequest` calls `LoadBaseInfo(&pRequest->BaseInfo)` | `src/SO3World/Src/KRelayClient.cpp:881` | candidate entry handoff exists |
| `LoadExtRoleData` passes only `(data, length)` to `LoadStateInfo` | `src/SO3World/Src/KPlayer.cpp:1992` | candidate lacks the target version dispatch at this call boundary |
| `LoadBaseInfo` reads `CurrentPos.dwMapID` for `GetScene` | `src/SO3World/Src/KPlayer.cpp:2489` | this is the first map-entry-sensitive consumer |
| `KRoleBaseInfo` currently contains the 162-byte target tail and 33 representation slots | `include/Include/KRoleDBDataDef.h:69` | source/candidate declaration must still be compiled/probed before acceptance |

The current declaration resemblance does not prove packet/blob semantics or
the behavior of a historical role row.

## Graph result and limitation

The saved query `raw/graph-function-LoadStateInfo.json` returned only a
`PROVISIONAL_EXACT_QUALIFIED_NAME` reconciliation: source/candidate spelling
matches the target method family, but it contained no target call/decompile
observation for the exact unversioned query. This is honest missing coverage,
not a negative target fact. The direct DWARF and PyGhidra artifacts above are
the authority for this dossier.

The broad Graph type query for `KRoleBaseInfo` timed out after 25 seconds and
produced no result. No claim in this dossier depends on that timed-out query.

## Required next evidence before a patch

1. Pin the current Center target artifact and the exact `R2S_PLAYER_LOGIN_REQUEST`
   envelope/role-data section delivery for a stock and candidate arm.
2. Capture one selected-role transition with the role-data header and block
   headers (type, version, length), without recording personal payload data.
3. Prove which `rbtStateInfo` version reaches `LoadStateInfo` and compare its
   length with the target parser selected by that version.
4. Prove the candidate's compiled `sizeof(KRoleBaseInfo) == 162` and packet
   sizes at the Relay boundary before accepting any header/source change.
5. Give the complete atomic diff, including unknown-block preservation, to an
   opposite-model reviewer before a native build.

## Explicit non-claims

- This does not prove that the active candidate can enter a scene.
- This does not prove that any existing database blob is v2.5-compatible.
- This does not authorize renumbering all role blocks or accepting unknown
  blocks by truncation.
- This does not replace the transport card's real client packet capture.
