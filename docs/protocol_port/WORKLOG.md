# GS_CLIENT_PROTOCOL (server->client s2c) realign 2010 -> v246 — WORKLOG

Goal: make the 2010-rebuild server send the exact protocol ids the real v246 client (JX3Client.exe,
Sep-Nov 2012, at /Volumes/Data/909160_剑侠3/客户端/JX3) expects — unblocking ALL server->client
packets for the v246 client (currency-sync, and everything else), not just currency.

Method = EXPLOIT the matched artifacts (v246 debug server DWARF + v246 client) instead of inferring.

## [P1] Enum realign (DWARF-mechanical) — DONE, build ok=202, boot [OK]
- Extracted full v246 GS_CLIENT_PROTOCOL from SO3GameServerD DWARF: 323 entries, contiguous 0..322
  (tools: llvm-dwarfdump --name=GS_CLIENT_PROTOCOL -> /tmp/v246_gsproto.txt).
- Diff vs 2010 (229 entries):
  - 222 common packets — **order-IDENTICAL** in both enums (clean superset, no reordering).
  - 101 NEW in v246 (inserted throughout).
  - 7 in 2010 not in v246 (removed/renamed): s2c_account_kickout (->_notify), s2c_over_time_action,
    s2c_sync_battle_field_list, s2c_sync_battle_field_side, s2c_sync_current_prestige (prestige moved
    to currency in v246), s2c_apex_protocol, s2c_sync_apply_exterior_flag.
- Regenerated the enum = v246 verbatim (explicit `name = value`, 0..322) + the 7 legacy-only names
  appended at 323..329 (so 2010 server code that still references them compiles; the v246 client just
  ignores those ids). Spliced into include/Include/GS_Client_Protocol.h (struct defs untouched).
- Key results: s2c_sync_currency = 207, s2c_sync_player_designation = 257 (v246 values).
- **Why safe / why it FIXES rather than breaks:** server code sends packets BY NAME
  (`Send(..., s2c_xxx)`); renumbering is transparent to the source but changes the wire byte to the
  v246 value. The 2010 wire bytes were ALREADY misaligned vs the v246 client (drift +37 by camp_info,
  +94 by end) — this realign makes them match. Enum is s2c-only (0 c2s_ entries; c2s is a separate enum).
- Verify so far: build clean (all 229 old names still resolve => no dropped reference), boot settings-[OK].
  LIVE verify (v246 client renders the packets) = P3 (client-oracle bring-up).

## Remaining
- **P2 struct drift**: for packets the server EMITS, reconcile struct layout vs v246 DWARF (id is now
  right; body must match too). Only emitted packets matter.
- **c2s (client->server) protocol**: separate enum — check if it also drifted (needed for the v246
  client's REQUESTS to parse: login/move/buy). If drifted, its own realign.
- **P3 client-oracle**: run v246 debug server (SO3GameServerD) + JX3Client.exe as a reference pair;
  capture/diff wire; then connect the client to the rebuild cluster to verify end-to-end.
