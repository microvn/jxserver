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

## [P1b] c2s (CLIENT_GS_PROTOCOL) realign + sentinel fix — DONE, build ok=202, boot [OK]
Second enum in the same file = CLIENT_GS_PROTOCOL (client->server, server PARSES). Same clean drift:
2010=146, v246=210; 134 common order-IDENTICAL; 76 new; 12 removed (c2s_apply_player_bufflist,
c2s_request_target, c2s_apex_protocol, tong-salary/repertory, camp, game-card, ...). c2s_handshake_request=1 both.

**Array-sizing fix (critical):** the server sizes inbound/stat tables by the enum sentinel:
`m_ProcessProtocolFuns[client_gs_connection_end]`, `m_nProtocolSize[client_gs_connection_end]`,
`m_DownwardProtocolStatTable[gs_client_connection_end]`. Legacy entries appended past the v246
sentinel would overflow those arrays. Fix: place legacy BEFORE the sentinel and bump the sentinel to
the true max -> gs_client_connection_end=329 (was v246 322 + 7 legacy), client_gs_connection_end=221
(was 209 + 12 legacy). Sentinel is server-internal (array size + inbound bound `id < end`); bumping is
safe (bigger table, looser bound; the v246 client carries its own enum). Verified build+boot.

Both directions (s2c + c2s) now speak v246 numbering. This is the infrastructure unblock for using the
real v246 client. Next: P2 struct-drift for emitted packets (id now right; body must match v246 too),
starting with DoSyncCurrency; P3 live client-oracle (JX3Client.exe) end-to-end verify.

## [P2a] External protocol-id widen BYTE->WORD — DONE (build+boot), NEEDS live-verify
Root cause found (S2C_SYNC_CURRENCY DWARF = 11B: DOWNWARDS_PROTOCOL_HEADER(2B) + byType@2 + nValue@3
+ nRemainSpace@7). v246 EXTERNAL_PROTOCOL_HEADER byte_size=2: **byProtocolID widened BYTE->WORD**
because v246 has >255 client protocols (s2c_sync_player_designation=257, s2c_sync_currency=207, ...).
The 1-byte 2010 id CANNOT hold them -> the enum realign (P1/P1b) is only correct WITH this widen.
- Changed EXTERNAL_PROTOCOL_HEADER.byProtocolID BYTE->WORD (SO3ProtocolBasic.h). Inherited by
  DOWNWARDS (s2c) + UPWARDS (c2s) -> both external headers now 2-byte id (matches the v246 client).
- Safe mechanically: send is field-based (`Pak.byProtocolID = s2c_xxx; Send(&Pak, sizeof(Pak))`) so the
  full 2-byte id goes on the wire; framing is length-prefixed (id-agnostic); dispatch reads the field.
- Verified: build ok=202, boot settings-[OK]. **NOT wire-verified** — this is a systemic wire change;
  correctness with the real client (and no residual 1-byte-assuming code) needs the live client-oracle.

## STATE: code-level v246 client-protocol alignment COMPLETE (enum s2c + enum c2s + id widen).
Committed to the v246 client (JX3Client.exe) as the target. This changes the wire for ALL client
packets; a 1-byte-header client would no longer work (intended — we target v246).

## P3 (live client-oracle) is now the MANDATORY next step, not optional
Only a live JX3Client.exe <-> server session verifies the wire (enum + header + per-packet struct drift).
Plan: stand up JX3Client.exe (Windows/Wine or the packaged launcher) pointed at the cluster gateway;
login; observe. The v246 debug server (SO3GameServerD) can serve as a reference pair for packet diff.
Remaining code work AFTER a green live login: P2b per-packet struct-drift for emitted packets (bodies),
DoSyncCurrency emit (id 207 now valid, struct known), then currency renders on the client.

## [S3 DONE] DoSyncCurrency emit + hook — currency now syncs to client (static-verified)
Unblocked by the protocol realign. S2C_SYNC_CURRENCY added to GS_Client_Protocol.h (header(2)+byType+
nValue+nRemainSpace); compiled sizeof=11 == v246 DWARF (struct-parity confirmed statically, no client).
- KPlayerServer::DoSyncCurrency (id s2c_sync_currency=207) + DoSyncCurrencyList (6 currencies).
- Hooked at every change site: KCurrency::AddCurrency / AddRemainSpace / Activate (only when a reset
  actually fires) -> DoSyncCurrency; shop BuyItem auto-syncs via AddCurrency. Initial sync on enter:
  KPlayer.cpp DoSyncCurrencyList next to DoSyncCoin/DoSyncDesignationData.
Verify: build ok=202, boot [OK], sizeof==11 (id+struct == v246). Live client-render = P3 (needs client).
