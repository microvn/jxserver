# Deep target decompile audit — role-load / 100% loading stall

Status: target evidence only. No candidate source change is authorized by this
file.

## Artifact

- Binary: `jx3_dwarf/SO3GameServerD`
- Binary SHA-256: `47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`
- Deep export: `raw/audit-20260728/target-role-load-deep.jsonl`
- Export SHA-256: `5d3aca03b9e4a8735c2a553e2e590f32be0c34e0e53423e19f8d76fd1ad11949`
- Export method: `compare-engine/tools/extract/pyghidra_export.py`, no-analysis,
  xrefs and CFG enabled, targeted decompilation.

## Confirmed target execution closure

### `KRelayClient::OnLoadRoleData @ 0x080e110c`

The target performs this order:

```text
role envelope validation
  -> GetObj(KPlayer)
  -> GetScene
  -> AddPlayer
  -> KPlayer::Load(data, length)
  -> failure abort if Load returns false
```

Therefore `DoSyncRoleDataOver` and later ready signalling cannot happen when
`KPlayer::Load` fails.

### `KPlayer::PartialLoadExtData @ 0x0839b62e`

The target treats role data as a sequence of 12-byte block headers:

```text
header: type (DWORD), version (DWORD), length (DWORD)
payload: length bytes
```

Confirmed behavior:

1. Reject fewer than 12 remaining bytes.
2. Reject a payload length larger than the remaining section.
3. Dispatch by `type`.
4. For `rbtState`/type `5`, call:

```text
KPlayer::LoadStateInfo(payload, length, version)
```

5. Advance by `length + 12` only after the block succeeds.
6. Send `DoSyncRoleDataSectionCheckRequest` for the processed block.
7. Mark the complete gate only after the final section acknowledgement.

The target call at the state block is visible in the decompile around
`0x0839bc5b`; the version argument is the second header word, not an inferred
source field.

### `KPlayer::LoadStateInfo @ 0x0839b112`

The target dispatches only versions `0`, `1`, and `2`:

| Version | Target parser | Minimum consumed payload |
|---:|---|---:|
| 0 | `LoadStateInfo_V0 @ 0x0839ab6a` | `0x75` bytes |
| 1 | `LoadStateInfo_V1 @ 0x0839a5b0` | `0x80` bytes |
| 2 | `LoadStateInfo_V2 @ 0x0839a01c` | `0x80` bytes |

Each parser rejects a payload shorter than its required size and rejects
unconsumed bytes. Other versions fail through the dispatcher.

### `KPlayer::OnExtDataLoadFinish @ 0x0839fb50`

The target checks the ext-data completion flag before emitting the final sync
fan-out. It then calls `DoSyncRoleDataOver` and only afterwards performs the
remaining client sync/login-script work. This confirms that the 100% client
stall is upstream of the final ready transition when role loading fails.

## Root-cause localization

The live candidate log reported:

```text
LoadStateInfo: uLeftSize == 0 failed
LoadExtRoleData failed
Role data error
```

This is consistent with the target contract and localizes the current stall to
the candidate role-state parser. It is not evidence of a client loading-panel
bug, and it is not fixed by broad client packet filtering.

## Candidate divergence

- Candidate `LoadStateInfo` accepts only `(data, length)`.
- Candidate `LoadExtRoleData` does not pass `KRoleBlockHeader::dwVer`.
- Candidate uses the source-era `KROLE_STATE_INFO` struct and a global
  `sizeof`/zero-remainder check.
- Candidate does not reproduce the target's per-block section-check gate.

## Safe next implementation boundary

The next patch may implement the target-backed version dispatcher and pass the
block version. Field mapping must use the three target parser decompilations;
it must not be reconstructed from source declaration order. Unknown target
blocks must remain observable/preservable, and the final ready response must
remain downstream of successful block processing and section acknowledgement.

Runtime acceptance requires the candidate log to lose `Role data error` and to
emit `s2c_sync_role_data_over` before any claim that the loading screen is
fixed.

## GraphEngine reconciliation after ingest

The deep export was ingested as an immutable supplement after a database backup
at `compare-engine/.local/backups/roleload-deep-20260728/evidence.sqlite.before`.
The evidence query now returns a target decompile document for
`KPlayer::LoadStateInfo` with evidence ID `30036`; the query no longer reports
the target decompile as absent. The raw export remains the re-openable authority
and the database stores only its hash/evidence reference.
