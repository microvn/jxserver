# R1 static contract — Relay enum and section-check request

Date: 2026-07-27  
Producer: Codex  
Scope: the two `REQUEST_CHANGES` blockers from the B1--B4 review only.

## Isolated source

- Base: canonical P3 commit `1516b8b0832bb071f9d3cc0e773136a6e0962208`.
- Worktree: `.worktrees/overlay-w1-entry-r1`.
- Historical B1--B4 raw overlay retained exactly; R1 adds only the corrections below.
- Pre-edit copies are under `evidence/r1-preedit/`.

## Target authority

Target artifact: `jx3_dwarf/SO3GameServerD`, SHA-256
`47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`.

`llvm-dwarfdump --name=KS2R_PROTOCOL -c` reports:

| Enumerator | Target value | Candidate R1 value |
|---|---:|---:|
| `s2r_search_map_queue_request` | 7 | 7 |
| `s2r_remote_talk_message` | 31 | 31 |
| `s2r_sync_fellowship_player_mini_avatar` | 39 | 39 |
| `s2r_load_role_data_request` | 57 | 57 |
| `s2r_load_account_data_request` | 58 | 58 |

`llvm-dwarfdump --name=S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST -c` reports:

- `DW_AT_byte_size = 3`;
- inherited `DOWNWARDS_PROTOCOL_HEADER` at offset `0` (the target external
  header is `WORD`); and
- `bySectionType` at offset `2`.

## R1 source change

1. Inserted the three target enum placeholders at their target ordinal
   positions. This preserves the required role/account IDs; it does not claim
   that an unreferenced placeholder route has an implementation closure.
2. Moved the locally declared section-check request into
   `GS_Client_Protocol.h` and wrapped only that packet in `#pragma pack(1)` /
   `#pragma pack()`. `KPlayerServer` now sends the named header type.

## Static checks

- `check_legacy_bytes.py` passed for all three edited mixed-encoding files:
  non-ASCII bytes and newline style are unchanged.
- `git diff --check` passed in the R1 worktree.
- Source enum walk reports `7, 31, 39, 57, 58` for the five target anchors.

## Deliberate non-changes

- No change to the `m_bSyncDataOver` / ready-gate behaviour.
- No claim that the account parser, account completion, ext-data producer, or
  initial-sync ordering is target-complete.
- No build, deployment, active-binary, or client-runtime claim.

## Next gate

Opposite-family review of this R1 diff against the canonical P3 baseline and
the exact target DWARF facts above. Build may start only if the review accepts
the bounded correction.
