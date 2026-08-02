# W1 static closure matrix

Purpose: one target-first research pass for every required W1 card. This is
the implementation queue, not a claim that the integrated client journey has
passed. Target identities are `SO3GameServerD`
`47716c73e8de281c95759cdc4a478e70e7c61322fb46e8c4e04954e51124b94a` and,
for Center peer contracts, `SO3GameCenterD`
`655b28126b972d1b3e1b3c02b827356c4d2ad6cf421b258152b130ec3c214d79`.

## Wave-wide findings

| Card | Target closure now pinned | Candidate status | Implementation decision |
|---|---|---|---|
| `FND-RUNTIME-INITIALIZATION` | `KSO3World::Init @ 0x0818f592`, settings/script/AI/skill roots and target boot fingerprint | combined candidate previously reaches loader/startup/map milestones | no new code batch; close provenance/review around the existing closure |
| `FND-CLIENT-GS-SECURE-TRANSPORT` | target security stream uses `ConnectSecurity`/`AcceptSecurity` and encode/decode mode | combined candidate was built with real `libcommon.a` | no speculative socket code; Windows capture decides whether this closure still drifts |
| `FND-RELAY-ROLE-ENVELOPE` | Center `DoPlayerLoginRequest @ 0x08084436`; typed request is 302 bytes, `BaseInfo @ +0x82`; response is 38 bytes | GS already registers `r2s_player_login_request` with 302 | preserve current typed envelope; capture framing before changing it |
| `FND-ROLE-LOAD-SPINE` | `LoadBaseInfo @ 0x0839dce2`; `LoadStateInfo(data,len,version) @ 0x0839b112`; target namespace has 49 block tags | candidate loads state without version and handles a smaller tag range | port as part of the ext-data data-plane batch, not as an isolated enum renumbering |
| `PLY-ENTER-WORLD` | account request/load, role load, ext-data section loop, ready ACK and `gsPlaying` gate now have direct target decompile | candidate has only old enter-scene path and direct completion ownership | implement bottom-up in B1–B4 below |

## Directly recovered entry-data contract

```text
OnApplyEnterScene @ 0x0807a300
  when gsWaitForLoginLoading (5):
    DoLoadAccountDataRequest(roleID, account)   protocol 58
    DoLoadRoleDataRequest(roleID)
    m_nTimer = 0
    state = gsWaitForRoleData (3)

OnLoadAccountData @ 0x080e0f06
  requires role/offset/success and state == 3
  → KPlayer::LoadAccountData

OnLoadRoleData @ 0x080e110c
  requires role/offset/success and state == 3
  → AddPlayer → KPlayer::Load

role/ext-data buffer
  → PartialLoadExtData @ 0x0839b62e
  → S2C section-check request (ID 153, 3 bytes)
  → C2S section-check response (ID 4, 6 bytes)
  → OnSyncRoleDataSectionCheckRespond @ 0x08079c9a
  → next PartialLoadExtData, until the final block is acknowledged
  → m_bExtDataLoadFinish = true
  → next section-check response invokes OnExtDataLoadFinish @ 0x0839fb50

C2S_CLIENT_CONFIRM_READY (ID 2, 6 bytes)
  → OnClientConfirmReady @ 0x08079dfc
  → OnClientReady @ 0x0839f87e
  → gsPlaying (7)
```

## Exact serialized contracts

| Contract | Target fact | Authority |
|---|---|---|
| account request | `s2r_load_account_data_request = 58`; typed buffer is 6 bytes: internal header + role ID | DWARF enum + `DoLoadAccountDataRequest` decompile |
| account response | `r2s_load_account_data = 64`; header fields: role ID `+2`, success `+6`, total length `+10`, data `+14` | DWARF `R2S_LOAD_ACCOUNT_DATA` |
| role response | same header shape, `R2S_LOAD_ROLE_DATA`, 14-byte fixed header | DWARF |
| section request | `S2C_SYNC_ROLE_DATA_SECTION_CHECK_REQUEST`: ID 153, 3 bytes, block type `+2` | DWARF + direct decompile |
| section response | `C2S_SYNC_ROLE_DATA_SECTION_CHECK_RESPOND`: ID 4, 6 bytes | DWARF |
| ready request | `c2s_client_confirm_ready`: ID 2, 6 bytes | DWARF + constructor/handler evidence |
| role block header | 12 bytes: type, version, length; state block is parsed with its version | direct `PartialLoadExtData` decompile + `LoadStateInfo` target signature |

## ABI/state facts needed by code

| Target field/state | Target value | Source consequence |
|---|---:|---|
| `gsWaitForRoleData` | 3 | preserve existing semantic state |
| `gsWaitForSyncClientData` | 4 | insert before source-era `gsWaitForLoginLoading` | 
| `gsWaitForLoginLoading` | 5 | all later status values shift by one; audit every serialized/status comparison in owned files |
| `gsPlaying` | 7 | do not set directly from candidate `OnLoadRoleData` |
| `KPlayer::m_piExtDataBuffer` | target member, offset `0xa088` | add as source member with normal C++ ownership; do not copy raw offset |
| `m_uNextLoadOffset` / `m_uNextLoadIndex` | `0xa08c` / `0xa090` | incremental 12-byte block parser state |
| `m_bExtDataLoadFinish` | `0xa094` | set only after the final ext-data block has been acknowledged |
| `m_nVirtualFrame` | `0xa098` | reset during client-ready |
| `m_eGameStatus` | `0xd00` | source enum is the contract; raw member offset is verification only |
| Relay account buffer/offset | `m_pbySyncAccountBuffer @ 0x4ef8`; `m_uSyncAccountOffset @ 0x4efc` | required to receive account chunks safely |

## B2 direct target closure update

Direct DWARF and PyGhidra audit completed before B2 source changes:

- Target `ROLE_DATA_BLOCK_TYPE` is contiguous `0..48` (`rbtTotal = 49`).
  Source and target agree only through `rbtMentorData = 22`; target then has
  `Pendent=23`, `ActivityVariables=24`, `Currency=25`, `BankPassword=26`,
  `Arena=27`, `DropSurpriseMask=28`, `CampActiveStat=29`, Exterior pair
  `30/31`, Hair `32`, Regression `33`, item/dungeon extensions `34..39`,
  FacePendent `40`, FellowPet `41`, DynamicPackage `42`, Tong `43`, Talent
  `44`, MiniAvatar `45`, VisitTong `46`, Rewards `47`, ManualDrop `48`.
- `PartialLoadExtData @ 0x0839b62e` processes exactly one 12-byte block from
  the retained role buffer, then sends section-check ID 153. It advances the
  offset only after that block is accepted; it releases the buffer and sets
  `m_bExtDataLoadFinish` only after the final block has been sent for ACK.
- `LoadStateInfo @ 0x0839b112` takes `(data, length, version)` and dispatches
  V0/V1/V2; versions above 2 fail. Source's two-argument loader is therefore
  not an equivalent target contract.

This is a source-migration requirement, not permission to renumber source
blocks blindly: B2 must reconcile every current source save/load producer with
the target number before changing the enum.

## Source batches — order is mandatory

### B1 — account Relay transport and parser

Own: `KRelayClient.{h,cpp}`, `Relay_GS_Protocol.h`, `KPlayer.{h,cpp}`.

- Add target protocol IDs 58/64 and packed response layout.
- Add account receive buffer/offset lifecycle to `KRelayClient`.
- Implement request/send and offset/role/success checks.
- Implement `KPlayer::LoadAccountData` framing/CRC/block walk, but only the
  target-proven account-state and regression blocks; retain explicit unknown
  block policy.

Gate: source counterpart of every account-state field must be inventoried
before writing `LoadAccountStateInfo`; account blob must not be silently
ignored.

### B2 — versioned role/ext-data ingestion

Own: `KPlayer.{h,cpp}`, `KRelayClient.cpp`, `KPlayerServer.{h,cpp}` and the
role protocol/type headers already owned by the card.

- Introduce `gsWaitForSyncClientData` and adjust later enum values.
- Move candidate's premature direct `gsPlaying` completion out of
  `OnLoadRoleData`.
- Add ext-data buffer state and the incremental 12-byte role-block parser.
- Pass header version into `LoadStateInfo`; preserve/defer target-only blocks
  explicitly rather than dropping them.

Gate: preserve the existing `KPlayer::Load` owner where target uses it; do not
port the 40-plus `OnExtDataLoadFinish` fan-out in this batch.

### B3 — section-check completion bridge

Own: `GS_Client_Protocol.h`, `KPlayerServer.{h,cpp}`, `KPlayer.{h,cpp}`.

- Register ID 4 with its exact six-byte response shape.
- Emit ID 153 after every accepted role-data block.
- Implement `OnSyncRoleDataSectionCheckRespond` and set
  `m_bExtDataLoadFinish` only after the final block/acknowledgement.
- Provide a minimal target-backed `OnExtDataLoadFinish` that performs the
  required role-data-over/script boundary, with every omitted target fan-out
  recorded as deferred to its owning feature card.

Gate: source ownership of `DoSyncRoleDataOver`, scene script and login script
must be reconciled so they are invoked once.

### B4 — client-ready/initial ACK bridge

Own: `GS_Client_Protocol.h`, `KPlayerServer.{h,cpp}`, `KPlayer.{h,cpp}`.

- Register ID 2, size 6, to `OnClientConfirmReady`.
- Require ext-data completion and `gsWaitForSyncClientData`.
- Implement the small `OnClientReady` state/region validation bridge.
- Register and implement the target `OnApplyCharacter` and initial object ACK
  handlers only after the matching S2C packet surfaces are confirmed in the
  candidate closure.

Gate: no fake completion flag, no duplicate `AddPlayer`, no second login script
or initial sync fan-out.

## What remains runtime-only

- actual client/GS security frame and first decoded packet;
- Center wire framing around typed 302-byte login envelope;
- stock role/account chunk ordering and which optional data blocks are present;
- final client scene-ready behavior.

These are acceptance measurements, not reasons to postpone B1–B4 static port
work. Each batch must still pass build and opposite-model diff review before
deployment.

## Raw target evidence saved by this pass

- `entry/raw/w1-entry-data-target.jsonl`: batch PyGhidra export containing
  target decompiles for entry, account, role, section-check and ready roots.
- DWARF probes in the Wave state/dossiers pin the protocol layouts, state enum,
  KPlayer ext-data fields and Relay account-buffer fields.
