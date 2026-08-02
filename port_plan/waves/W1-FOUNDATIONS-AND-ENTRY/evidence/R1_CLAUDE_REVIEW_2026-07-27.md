# R1 opposite-family review

## Verdict

`PASS WITH CAVEATS`

Claude Code completed a read-only review of the exact R1 overlay diff
`05e6094f6b6d7e9c64578837cdd4d613d29c9268ea0cff5a06eece6ddaf90b86`
against canonical P3 commit `1516b8b0832bb071f9d3cc0e773136a6e0962208`.

## Confirmed

- Target DWARF confirms the five repaired `KS2R_PROTOCOL` anchors: 7, 31,
  39, 57, and 58.  The R1 header computes those values exactly.
- Target DWARF gives `S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST` size 3 and
  `bySectionType` offset 2.  The R1 declaration is header size 2 plus byte 1.
- `#pragma pack(push, 1)` / `#pragma pack(pop)` is balanced within the outer
  header `pack(1)` region, preserving the packed ABI of all following structs.
- The candidate confirm-ready and section-check response IDs match the target
  enums; no duplicate existing external registration was found.

## Caveats retained

- Relay enum drift at IDs 65 and above is pre-existing and outside this entry
  slice; future senders in that range must not be assumed correct.
- Ready/account state semantics and runtime entry parity remain unproven.
- This review is static only. It authorizes an isolated native build, not
  deployment or Wave acceptance.
