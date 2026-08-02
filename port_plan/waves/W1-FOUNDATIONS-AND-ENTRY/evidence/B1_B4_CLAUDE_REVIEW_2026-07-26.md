# B1--B4 overlay review — Claude

## Scope

- Reviewer: Claude (foreground TUI; opposite model to Codex producer).
- Candidate: `.worktrees/overlay-w1-entry` at canonical baseline
  `1516b8b0832bb071f9d3cc0e773136a6e0962208` plus the exact 11-file overlay.
- Overlay raw-diff SHA-256:
  `c479f26a0d5c96b3d123b4fbdefdd43d058e9513c01dd092a783ef637934f23f`.
- Review mode: read-only. No source edit, build, deployment, restart, or client
  login was performed.

## Verdict

`REQUEST_CHANGES` — do not build or deploy this overlay as-is.

The reviewer confirmed valuable target-aligned ABI work: `KACCOUNT_STATE_INFO`
(76 bytes), `KROLE_POSITION_DB` with `nCenterIndex`, the 162-byte
`KRoleBaseInfo` layout adjustment, `gsWaitForSyncClientData = 4`, and the
account Relay packet layouts that were directly matched to target DWARF.

Those facts do **not** make the integrated B1--B4 overlay acceptable.

## Blocking findings

1. **B-1 — HIGH, high confidence**: the local
   `S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST` is declared inside a function
   without the packed protocol region. Its natural size is 4 bytes, while target
   DWARF says 3 bytes. The client would receive an extra padding byte at the
   section-ready boundary. Location: `KPlayerServer.cpp:5611`.
2. **B-2 — HIGH, high confidence**: candidate `KS2R_PROTOCOL` resolves
   `s2r_load_role_data_request`/`s2r_load_account_data_request` to 54/55 while
   target DWARF resolves them to 57/58. The pre-existing enum is already two
   entries short before this point, so neither request reaches its target Center
   route. Location: `Relay_GS_Protocol.h:98`.
3. **B-3 — HIGH, high confidence**: `OnPlayerLoginRequest` removes the
   `m_bSyncDataOver` consumer while the overlay also reconnects its producer in
   `OnSyncMentorData`; `OnTransferPlayerRequest` still retains the gate. This is
   an unproven, asymmetric ready-gate semantic change. Location:
   `KRelayClient.cpp:878`.
4. **B-4 — MEDIUM, high confidence**: `KPlayer::LoadAccountData` maps account
   blocks using unsourced integer cases. The evidence dossier itself marks the
   account block contract unresolved; a 76-byte account-state payload can be
   rejected by the 22-byte regression parser. Locations: `KPlayer.cpp:1836,1839`.
5. **B-5 — MEDIUM, medium confidence**: account and role requests are issued
   back-to-back but `OnLoadAccountData` requires `gsWaitForRoleData`; a role
   response arriving first can move the player to `gsWaitForSyncClientData` and
   drop the account response. Location: `KRelayClient.cpp:2337`.

The review also records lower-severity hygiene findings (literal `\\n`, legacy
whitespace/newline drift) and an existing, out-of-overlay `R2S_PLAYER_LOGIN_REQUEST`
layout concern. They are not grounds to change canonical P3 during this review.

## Required next action

1. Correct/identify the two missing `KS2R_PROTOCOL` entries, then pin the full
   target name/value table before touching account-route IDs. Direct DWARF
   recheck after review confirms role/account request IDs are 57/58; the
   reviewer's prose had an off-by-one and must not be used as authority.
2. Move the 3-byte section-check wire type into the packed protocol header (or
   send exactly three bytes), then verify its compiled size.
3. Re-split or re-evidence the ready-gate/state changes separately from the ABI
   and Relay-ID corrections; do not remove a gate merely to pass entry.
4. Decompile `KPlayer::LoadAccountData @ 0x08399acc` to identify the target
   account block enum and parser before implementing block cases.

No runtime conclusion is drawn from this review.
