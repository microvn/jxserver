# W1 enter-world static dossier

## Scope and limit

This dossier covers the v2.5 entry acknowledgement path only.  It was opened
while no Windows client is available.  The working hypothesis is deliberately
narrow: authentication and role selection reach the GameServer, then entry
does not reach scene-ready.  It is a research constraint, **not** a measured
runtime fact and not an acceptance result.

The target authority is `SO3GameServerD`, SHA-256
`47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a`.

## Confirmed target entry closure

| Target entity | Address | Target-backed behaviour | Evidence |
|---|---:|---|---|
| `KPlayerServer::OnClientConfirmReady` | `0x08079dfc` | Requires a player, `m_bExtDataLoadFinish`, and `gsWaitForSyncClientData`; calls `KPlayer::OnClientReady`; otherwise logs and detaches the connection. | DWARF symbol, target call closure, decompile raw file |
| `KPlayer::OnClientReady` | `0x0839f87e` | Sets virtual frame/record count, transitions to `gsPlaying`, validates the current scene region, then applies the target Tong cleanup/update condition. | DWARF symbol, target call closure, decompile raw file |
| `KPlayerServer::OnApplyCharacter` | `0x08061eb0` | Consumes a ten-byte `C2S_APPLY_CHARACTER` (`dwCharacterID @ +6`) and schedules a visible player/NPC object only when the target is valid and in range. | DWARF `C2S_APPLY_CHARACTER`, target decompile raw file |
| `KPlayerServer::OnSyncNewPlayerRespond` | `0x08058424` | Advances `m_uSyncPlayerSN` only on matching `wSyncSN`; copies the three response counts. | DWARF type + target decompile raw file |
| `KPlayerServer::OnSyncNewNpcRespond` | `0x08058398` | Advances `m_uSyncNpcSN` only on matching `wSyncSN`; copies one response count. | DWARF type + target decompile raw file |
| `KPlayerServer::OnSyncNewDoodadRespond` | `0x0805830c` | Advances `m_uSyncDoodadSN` only on matching `wSyncSN`; copies one response count. | DWARF type + target decompile raw file |

The target has both `OnApplyEnterScene` and the above newer acknowledgement
handlers.  This dossier does **not** infer that the older route may be
removed; route ordering and the state transition into
`gsWaitForSyncClientData` still require a target decompile/capture.

## Target wire facts

| Route / type | Target fact | Authority |
|---|---|---|
| `c2s_client_confirm_ready` | enum value `2`; header-only request is consistent with the handler not reading `pData`. | DWARF enum DIE `0x002543fa` + decompile |
| `c2s_apply_character` | enum value `183`; `C2S_APPLY_CHARACTER` size `10`, `dwCharacterID @ +6`. | DWARF enum DIE `0x002548af`; type DIE `0x0032ad97` |
| `C2S_SYNC_NEW_PLAYER_RESPOND` | size `11`; `wSyncSN @ +6`, `uSyncCount[3] @ +8`. | DWARF DIE `0x0032aeca` |
| `C2S_SYNC_NEW_NPC_RESPOND` | size `9`; `wSyncSN @ +6`, `uSyncCount @ +8`. | DWARF DIE `0x0032aeff` |
| `C2S_SYNC_NEW_DOODAD_RESPOND` | size `9`; `wSyncSN @ +6`, `uSyncCount @ +8`. | DWARF DIE `0x0032af34` |

## Additional drift found in the old entry route

The target still contains `KPlayerServer::OnApplyEnterScene` at
`0x0807a300`; the newer acknowledgement handlers do not replace it.  Its
decompile proves two differences that are currently absent from the candidate
route:

| Claim | Target evidence | Candidate evidence | Consequence |
|---|---|---|---|
| Login-loading requests account data before role data | target `OnApplyEnterScene` decompile: `KRelayClient::DoLoadAccountDataRequest(dwRoleID, m_szAccount)` followed by `DoLoadRoleDataRequest(dwRoleID)` | `KPlayerServer.cpp:7698` calls only `DoLoadRoleDataRequest`; `KRelayClient.h/.cpp` expose no `DoLoadAccountDataRequest` | account/ext-data completion may never be produced; this is a concrete role-load dependency, not a guess |
| Scene timestamp is read from the request at `pData + 6` | target `OnApplyEnterScene` decompile at `0x0807a300` | candidate also reads `pApply->nSceneTimeStamp` in `KPlayerServer.cpp:7689` | this dimension matches structurally; it is not the root cause by itself |
| Ready path is a second transition after external data completion | target `OnClientConfirmReady` requires `m_bExtDataLoadFinish` and `gsWaitForSyncClientData`, then calls `KPlayer::OnClientReady` | candidate has no matching handler/registration/state surface in `KPlayerServer.*` | candidate must not be repaired by changing only `OnApplyEnterScene`; the account-data and ready-ack closures are separate gates |

This is a **static target/source drift finding**.  It does not prove that the
missing account request alone causes the observed client error; the producer of
`m_bExtDataLoadFinish` and the stock/candidate packet order remain required.

## Historical-session audit: additional state-machine drift

The prior Wave 3 records add two important constraints to this audit:

1. The candidate's current `KRelayClient::OnLoadRoleData` (`KRelayClient.cpp:2330-2363`)
   performs `AddPlayer`, `Load`, `DoSyncRoleDataOver`, scene/login scripts, and then
   sets `m_eGameStatus = gsPlaying` directly.  The target has an intermediate
   `gsWaitForSyncClientData` gate and only reaches `gsPlaying` from
   `KPlayer::OnClientReady` after protocol 2.  This is a proven state-machine
   mismatch, not merely a missing function name.
2. The historical Wave 3 runtime captures identified separate upstream blockers
   (Gateway queue/Paysys handoff, a premature `m_bSyncDataOver` rejection,
   `KROLE_POSITION_DB` layout, and pass-through transport framing).  Those records
   must be treated as prerequisites already fixed or revalidated by the active
   candidate hash; they must not be re-labelled as the current scene-entry root
   cause without a fresh active-process log/packet observation.

The historical records also reported missing timeout/detach cleanup around the
ready bridge.  If a ready route is later implemented, cleanup and timeout paths
must be reviewed as part of the same state-machine contract; a successful ready
handler alone is insufficient.

## Five-item static audit results

### 1. Constructor registration

Target `KPlayerServer` constructor symbols are `0x08086dbc` and `0x08089e96`.
The earlier Wave 3 RE record pins the relevant registration to
`c2s_client_confirm_ready = 2` with a six-byte protocol entry.  The candidate
header retains enum value `2`, but its registration list has no handler for this
ID.  The omission is confirmed; the complete constructor table still needs a
byte/address-level export before any write.

### 2. `m_bExtDataLoadFinish` producer

Graph target evidence identifies:

```text
KPlayer::OnExtDataLoadFinish        0x0839fb50
caller: KPlayerServer::OnSyncRoleDataSectionCheckRespond 0x08079c9a
caller: KPlayer::Load                         0x083a02b8
```

`OnSyncRoleDataSectionCheckRespond` calls `PartialLoadExtData` and then
`OnExtDataLoadFinish`.  The candidate has no matching method, fields, or
registration.  This is a missing extended-data protocol/state closure, not
just a missing flag assignment.

### 3. Relay response gating ready

The target graph connects the ready prerequisite to the role-data
section-check response.  `OnExtDataLoadFinish` emits `DoSyncRoleDataOver` and
the v2.5 initial-sync fan-out; `OnClientConfirmReady` consumes the resulting
`m_bExtDataLoadFinish` and `gsWaitForSyncClientData` state.  The exact wire
response ID/size and candidate Relay registration remain unresolved.

### 4. `OnLoadRoleData` ownership comparison

Target `KRelayClient::OnLoadRoleData @ 0x080e110c` has the static call closure:

```text
GetObj → GetScene → AddPlayer → KPlayer::Load
```

Candidate `KRelayClient::OnLoadRoleData` additionally performs
`DoSyncRoleDataOver`, scene/login scripts, and directly sets `gsPlaying`.
Target ownership moves those operations into `OnExtDataLoadFinish` and later
`OnClientReady`.  This is a concrete ownership/state-order drift; adding a
second copy of the target fan-out would be unsafe.

### 5. Active candidate provenance

The local deploy tree currently contains stock `SO3GameServer` SHA
`3002bf4a...` and local `SO3GameServer_ours` SHA `7b887520...`.  Historical
Wave 3 candidate hashes are deployment records, not current local active
process proof.  Until VPS `/proc/<pid>/exe` is rechecked, we cannot claim that
the active candidate includes the mentor-sentinel, role-layout, transport, or
ready-bridge changes from those logs.

### 6. Section-check protocol is now target-pinned

DWARF gives the missing boundary exact wire facts:

```text
C2S_SYNC_ROLE_DATA_SECTION_CHECK_RESPOND
  size: 6 bytes
  base: UPWARDS_PROTOCOL_HEADER
  payload fields: none beyond the 6-byte header
  DWARF DIE: 0x0032853f

S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST
  size: 3 bytes
  bySectionType @ +2
  DWARF DIE: 0x003258ea

CLIENT_GS_PROTOCOL
  c2s_sync_role_data_section_check_respond = 4
```

The target graph closure also pins `DoSyncRoleDataSectionCheckRequest @
0x0805c2b8` as a callee of `PartialLoadExtData`, and
`OnSyncRoleDataSectionCheckRespond @ 0x08079c9a` as the server-side consumer.
The candidate source has the enum value but no corresponding protocol struct,
registration, `PartialLoadExtData`, or response handler.  This is the first
fully target-pinned missing bridge after the account-data finding.

### 7. Target `OnExtDataLoadFinish` is a large fan-out, not a minimal gate

The target dossier records 40+ direct callees, including
`DoSyncRoleDataOver`, `DoSyncCurrencyList`, `DoSyncPlayerStateInfo`,
`DoSyncCampInfo`, `DoSyncBuffList`, `DoSyncExteriorAllSetData`,
`DoSyncDesignationData`, `DoSyncAchievementData`, `DoSyncFellowPetData`, and
`CallLoginScript`.  The correct next unit is therefore a section-check/load
bridge plus an explicitly measured minimal fan-out, not a wholesale copy of
`OnExtDataLoadFinish`.

### 8. Out-of-the-box finding: account data is a separate spine

The target has a separate account-data path:

```text
DoLoadAccountDataRequest      @ 0x080cfe8a
  caller: OnApplyEnterScene   @ 0x0807a300
OnLoadAccountData             @ 0x080e0f06
  -> KPlayer::LoadAccountData @ 0x08399acc
```

`KPlayer::LoadAccountData` loads account-state and regression data. The
candidate has no `DoLoadAccountDataRequest`, no `OnLoadAccountData`, and no
`KPlayer::LoadAccountData` implementation; the only candidate occurrence is a
comment in `KRegressionPlayerData.cpp`. The entry path is therefore missing
two independent persistence inputs: account data and role/ext data. A role list
can still work while this later state machine is incomplete.

Account loading must not be treated as a side effect of role loading. It needs
its own target-pinned request/response and DB/blob evidence, then feeds the
extended-data completion gate.

### 9. Knowledge-base architecture cross-check

The project knowledge base identifies `player-core` as a mega-hub with 588
target-only/drift methods and a serialization spine of roughly 24 sub-blobs.
That explains why copying only the missing ready handlers would be unsafe: the
functions sit at the intersection of account data, role data, section
versioning, player state, and initial packet fan-out. The safe next unit is a
bounded **entry-data spine** (account request/load, section-check, completion,
ready gate), not a broad `KPlayer` port.

## Candidate comparison

The current candidate source contains only the older
`c2s_apply_enter_scene -> KPlayerServer::OnApplyEnterScene` route.  Its
`KPlayerServer` header, implementation, and registration list contain no
`OnClientConfirmReady`, `OnClientReady`, `OnApplyCharacter`, or
`OnSyncNew{Player,Npc,Doodad}Respond` surface.  It also has no corresponding
per-player acknowledgement fields.  This is a source/candidate observation,
not a claim that every missing target function must be copied unchanged.

The candidate already emits `s2c_sync_new_{player,npc,doodad}`.  Therefore a
missing response route is a credible entry/initial-sync failure candidate and
must be resolved before blaming table data, Lua, or the client UI.

## Port decision and open gates

**Provisional port unit:** one entry acknowledgement closure, owned by
`PLY-ENTER-WORLD`, spanning declarations, packet layouts/registrations,
per-player state, handlers, and the exact state transition.

No source write lease exists yet.  Before implementation:

1. Decompile target `OnApplyEnterScene` and the target constructor registration
   table enough to pin the state/order relationship between old and new routes.
   The old route is now captured; constructor registration and the target
   account-data producer remain open.
2. Compare target and candidate `KPlayer` field/constructor initialization;
   do not add fields by source intuition.
3. Pin the target `m_bExtDataLoadFinish` producer and the transition into
   `gsWaitForSyncClientData`.
4. Reconcile the candidate `OnLoadRoleData` direct `gsPlaying` transition
   against the target intermediate state; do not duplicate target fan-out until
   this ownership difference is resolved.
5. Create a card-scoped baseline manifest and a narrow write lease.
6. Have the opposite model family review the raw target evidence and patch.
7. When a client is available, run the paired stock/candidate select-role
   capture; it must prove first decoded packet, ready confirmation, entry
   state, and acknowledgement sequence.

## Raw evidence

All files below are local-only and have the SHA-256 values listed in the
Wave state checkpoint:

```text
raw/target-OnApplyCharacter.decompile.txt
raw/target-OnClientConfirmReady.decompile.txt
raw/target-OnClientReady.decompile.txt
raw/target-OnSyncNewPlayerRespond.decompile.txt
raw/target-OnSyncNewNpcRespond.decompile.txt
raw/target-OnSyncNewDoodadRespond.decompile.txt
raw/graph-OnApplyCharacter.json
raw/graph-OnClientConfirmReady.json
raw/graph-c2s_apply_character.json
raw/graph-c2s_client_confirm_ready.json
```
